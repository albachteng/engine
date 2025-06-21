#include <gtest/gtest.h>
#include "../include/SceneManager.hpp"
#include "../include/BaseScene.hpp"
#include "../include/InputEvent.hpp"
#include <memory>
#include <thread>
#include <chrono>
#include <vector>
#include <atomic>
#include <SFML/Window/Event.hpp>

/**
 * Comprehensive unit tests for thread-safe SceneManager
 * 
 * Tests specifically target the race condition fixes:
 * - Concurrent scene transitions
 * - Thread safety of getCurrentScene()
 * - Exception safety and rollback
 * - Deferred transition system
 * - Resource lifecycle management
 */

// Mock scene for testing
class MockScene : public BaseScene {
private:
    std::string m_name;
    mutable std::atomic<int> m_loadCalls{0};
    mutable std::atomic<int> m_unloadCalls{0};
    mutable std::atomic<int> m_updateCalls{0};
    bool m_shouldThrowOnLoad = false;
    bool m_shouldThrowOnUnload = false;

public:
    MockScene(const std::string& name) : m_name(name) {}
    
    // Test configuration
    void setShouldThrowOnLoad(bool shouldThrow) { m_shouldThrowOnLoad = shouldThrow; }
    void setShouldThrowOnUnload(bool shouldThrow) { m_shouldThrowOnUnload = shouldThrow; }
    
    // BaseScene implementation
    void onLoad() override {
        m_loadCalls++;
        if (m_shouldThrowOnLoad) {
            throw std::runtime_error("Mock scene load failure for " + m_name);
        }
        // Simulate load time
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
    
    void onUnload() override {
        m_unloadCalls++;
        if (m_shouldThrowOnUnload) {
            throw std::runtime_error("Mock scene unload failure for " + m_name);
        }
        // Simulate unload time
        std::this_thread::sleep_for(std::chrono::milliseconds(5));
    }
    
    void update(float) override { m_updateCalls++; }
    void processInput(const InputEvent&, float) override {}
    void sMovement(float) override {}
    void sInput(sf::Event&, float) override {}
    void sRender() override {}
    
    // Test accessors
    std::string getName() const { return m_name; }
    int getLoadCalls() const { return m_loadCalls; }
    int getUnloadCalls() const { return m_unloadCalls; }
    int getUpdateCalls() const { return m_updateCalls; }
};

class SceneManagerTest : public ::testing::Test {
protected:
    void SetUp() override {
        sceneManager = std::make_unique<SceneManager>();
        
        // Register mock scenes
        sceneManager->registerScene("Scene1", []() {
            return std::static_pointer_cast<BaseScene>(std::make_shared<MockScene>("Scene1"));
        });
        
        sceneManager->registerScene("Scene2", []() {
            return std::static_pointer_cast<BaseScene>(std::make_shared<MockScene>("Scene2"));
        });
        
        sceneManager->registerScene("Scene3", []() {
            return std::static_pointer_cast<BaseScene>(std::make_shared<MockScene>("Scene3"));
        });
        
        // Scene that throws during load
        sceneManager->registerScene("FailingScene", []() {
            auto scene = std::make_shared<MockScene>("FailingScene");
            scene->setShouldThrowOnLoad(true);
            return std::static_pointer_cast<BaseScene>(scene);
        });
    }

    void TearDown() override {
        sceneManager.reset();
    }

    std::shared_ptr<MockScene> getMockScene() {
        auto scene = sceneManager->getCurrentScene();
        return std::dynamic_pointer_cast<MockScene>(scene);
    }

    std::unique_ptr<SceneManager> sceneManager;
};

// ============================================================================
// Basic Functionality Tests
// ============================================================================

TEST_F(SceneManagerTest, Initialization_StartsWithNoScene) {
    EXPECT_EQ(sceneManager->getCurrentScene(), nullptr);
    EXPECT_EQ(sceneManager->getCurrentSceneState(), SceneState::INACTIVE);
    EXPECT_TRUE(sceneManager->getCurrentSceneName().empty());
    EXPECT_FALSE(sceneManager->isTransitioning());
    EXPECT_TRUE(sceneManager->isValid());
}

TEST_F(SceneManagerTest, RequestTransition_QueuesPendingTransition) {
    sceneManager->requestSceneTransition("Scene1");
    
    EXPECT_TRUE(sceneManager->isTransitioning()); // Has pending transition
    EXPECT_EQ(sceneManager->getCurrentScene(), nullptr); // Not processed yet
    
    auto stats = sceneManager->getStatistics();
    EXPECT_EQ(stats.pendingTransitions, 1);
}

TEST_F(SceneManagerTest, ProcessTransitions_LoadsQueuedScene) {
    sceneManager->requestSceneTransition("Scene1");
    sceneManager->processTransitions();
    
    auto scene = sceneManager->getCurrentScene();
    ASSERT_NE(scene, nullptr);
    
    auto mockScene = getMockScene();
    ASSERT_NE(mockScene, nullptr);
    EXPECT_EQ(mockScene->getName(), "Scene1");
    EXPECT_EQ(mockScene->getLoadCalls(), 1);
    EXPECT_EQ(sceneManager->getCurrentSceneState(), SceneState::ACTIVE);
}

TEST_F(SceneManagerTest, SceneTransition_UnloadsOldScene) {
    // Load first scene
    sceneManager->requestSceneTransition("Scene1");
    sceneManager->processTransitions();
    
    auto firstScene = getMockScene();
    ASSERT_NE(firstScene, nullptr);
    EXPECT_EQ(firstScene->getLoadCalls(), 1);
    
    // Transition to second scene
    sceneManager->requestSceneTransition("Scene2");
    sceneManager->processTransitions();
    
    auto secondScene = getMockScene();
    ASSERT_NE(secondScene, nullptr);
    EXPECT_EQ(secondScene->getName(), "Scene2");
    EXPECT_EQ(firstScene->getUnloadCalls(), 1); // First scene should be unloaded
}

// ============================================================================
// Thread Safety Tests (Race Condition Prevention)
// ============================================================================

TEST_F(SceneManagerTest, ConcurrentGetCurrentScene_ThreadSafe) {
    // Load initial scene
    sceneManager->requestSceneTransition("Scene1");
    sceneManager->processTransitions();
    
    std::atomic<int> nullReturns{0};
    std::atomic<int> validReturns{0};
    std::atomic<bool> shouldStop{false};
    
    // Thread 1: Continuously get current scene
    std::thread reader([&]() {
        while (!shouldStop) {
            auto scene = sceneManager->getCurrentScene();
            if (scene == nullptr) {
                nullReturns++;
            } else {
                validReturns++;
            }
            std::this_thread::sleep_for(std::chrono::microseconds(500));
        }
    });
    
    // Thread 2: Request rapid scene transitions
    std::thread writer([&]() {
        for (int i = 0; i < 5; ++i) {
            std::string sceneName = (i % 2 == 0) ? "Scene2" : "Scene1";
            sceneManager->requestSceneTransition(sceneName);
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
    });
    
    // Main thread: Process transitions with delays
    for (int i = 0; i < 10; ++i) {
        sceneManager->processTransitions();
        std::this_thread::sleep_for(std::chrono::milliseconds(20));
    }
    
    shouldStop = true;
    reader.join();
    writer.join();
    
    // getCurrentScene() should return valid scenes most of the time
    // Note: null returns are expected during scene transitions
    EXPECT_GT(validReturns.load(), 0);
    
    // After all transitions, should have valid scene
    auto finalScene = sceneManager->getCurrentScene();
    EXPECT_NE(finalScene, nullptr);
    EXPECT_EQ(sceneManager->getCurrentSceneState(), SceneState::ACTIVE);
}

TEST_F(SceneManagerTest, ConcurrentSceneRequests_ProcessedSafely) {
    const int numThreads = 4;
    const int requestsPerThread = 25;
    std::vector<std::thread> threads;
    
    // Launch multiple threads requesting different scenes
    for (int t = 0; t < numThreads; ++t) {
        threads.emplace_back([&, t]() {
            for (int i = 0; i < requestsPerThread; ++i) {
                std::string sceneName = "Scene" + std::to_string((t % 3) + 1);
                sceneManager->requestSceneTransition(sceneName);
                std::this_thread::sleep_for(std::chrono::milliseconds(1));
            }
        });
    }
    
    // Process transitions in main thread
    for (int i = 0; i < 200; ++i) {
        sceneManager->processTransitions();
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
    
    // Wait for all threads
    for (auto& thread : threads) {
        thread.join();
    }
    
    // Final processing
    for (int i = 0; i < 10; ++i) {
        sceneManager->processTransitions();
    }
    
    // Should end up with a valid scene
    auto finalScene = sceneManager->getCurrentScene();
    EXPECT_NE(finalScene, nullptr);
    EXPECT_EQ(sceneManager->getCurrentSceneState(), SceneState::ACTIVE);
    
    auto stats = sceneManager->getStatistics();
    EXPECT_GT(stats.totalTransitions, 0);
    EXPECT_TRUE(sceneManager->isValid());
}

// ============================================================================
// Exception Safety and Rollback Tests
// ============================================================================

TEST_F(SceneManagerTest, SceneLoadFailure_RollsBackSafely) {
    // Load valid scene first
    sceneManager->requestSceneTransition("Scene1");
    sceneManager->processTransitions();
    
    auto validScene = getMockScene();
    ASSERT_NE(validScene, nullptr);
    EXPECT_EQ(validScene->getName(), "Scene1");
    
    // Attempt to load failing scene
    sceneManager->requestSceneTransition("FailingScene");
    sceneManager->processTransitions();
    
    // Should still have original scene (rollback)
    auto currentScene = getMockScene();
    ASSERT_NE(currentScene, nullptr);
    EXPECT_EQ(currentScene->getName(), "Scene1");
    
    auto stats = sceneManager->getStatistics();
    EXPECT_GT(stats.failedTransitions, 0);
}

TEST_F(SceneManagerTest, LoadFailureFromEmptyState_HandlesGracefully) {
    // Attempt to load failing scene from empty state
    sceneManager->requestSceneTransition("FailingScene");
    sceneManager->processTransitions();
    
    // Should have no current scene
    EXPECT_EQ(sceneManager->getCurrentScene(), nullptr);
    EXPECT_EQ(sceneManager->getCurrentSceneState(), SceneState::FAILED);
    
    auto stats = sceneManager->getStatistics();
    EXPECT_GT(stats.failedTransitions, 0);
    
    // Should still be able to load valid scene after failure
    sceneManager->requestSceneTransition("Scene1");
    sceneManager->processTransitions();
    
    auto scene = getMockScene();
    ASSERT_NE(scene, nullptr);
    EXPECT_EQ(scene->getName(), "Scene1");
}

// ============================================================================
// Deferred Transition System Tests
// ============================================================================

TEST_F(SceneManagerTest, MultipleQueuedTransitions_ProcessedInOrder) {
    // Queue multiple transitions
    sceneManager->requestSceneTransition("Scene1");
    sceneManager->requestSceneTransition("Scene2");
    sceneManager->requestSceneTransition("Scene3");
    
    auto stats = sceneManager->getStatistics();
    EXPECT_EQ(stats.pendingTransitions, 3);
    
    // Process first transition
    sceneManager->processTransitions();
    auto scene1 = getMockScene();
    ASSERT_NE(scene1, nullptr);
    EXPECT_EQ(scene1->getName(), "Scene1");
    
    // Process second transition
    sceneManager->processTransitions();
    auto scene2 = getMockScene();
    ASSERT_NE(scene2, nullptr);
    EXPECT_EQ(scene2->getName(), "Scene2");
    
    // Process third transition
    sceneManager->processTransitions();
    auto scene3 = getMockScene();
    ASSERT_NE(scene3, nullptr);
    EXPECT_EQ(scene3->getName(), "Scene3");
    
    // No more transitions
    sceneManager->processTransitions();
    auto finalScene = getMockScene();
    ASSERT_NE(finalScene, nullptr);
    EXPECT_EQ(finalScene->getName(), "Scene3"); // Should still be Scene3
}

TEST_F(SceneManagerTest, DuplicateTransitions_SkippedCorrectly) {
    sceneManager->requestSceneTransition("Scene1");
    sceneManager->processTransitions();
    
    // Request same scene again
    sceneManager->requestSceneTransition("Scene1");
    
    auto stats = sceneManager->getStatistics();
    EXPECT_EQ(stats.pendingTransitions, 0); // Should be skipped
}

TEST_F(SceneManagerTest, ForceTransition_ReloadsCurrentScene) {
    sceneManager->requestSceneTransition("Scene1");
    sceneManager->processTransitions();
    
    auto scene1 = getMockScene();
    ASSERT_NE(scene1, nullptr);
    int initialLoadCalls = scene1->getLoadCalls();
    
    // Force reload same scene
    sceneManager->requestSceneTransition("Scene1", true);
    sceneManager->processTransitions();
    
    auto reloadedScene = getMockScene();
    ASSERT_NE(reloadedScene, nullptr);
    EXPECT_EQ(reloadedScene->getName(), "Scene1");
    
    // Should be a new instance (original would be unloaded)
    EXPECT_EQ(scene1->getUnloadCalls(), 1);
}

// ============================================================================
// Error Handling and Edge Cases
// ============================================================================

TEST_F(SceneManagerTest, RegisterInvalidScene_ThrowsException) {
    EXPECT_THROW(sceneManager->registerScene("", nullptr), std::invalid_argument);
    EXPECT_THROW(sceneManager->registerScene("ValidName", nullptr), std::invalid_argument);
}

TEST_F(SceneManagerTest, TransitionToUnregisteredScene_HandledGracefully) {
    sceneManager->requestSceneTransition("NonexistentScene");
    
    auto stats = sceneManager->getStatistics();
    EXPECT_EQ(stats.pendingTransitions, 0); // Should not be queued
}

TEST_F(SceneManagerTest, ClearPendingTransitions_RemovesQueuedTransitions) {
    sceneManager->requestSceneTransition("Scene1");
    sceneManager->requestSceneTransition("Scene2");
    sceneManager->requestSceneTransition("Scene3");
    
    auto stats = sceneManager->getStatistics();
    EXPECT_EQ(stats.pendingTransitions, 3);
    
    sceneManager->clearPendingTransitions();
    
    stats = sceneManager->getStatistics();
    EXPECT_EQ(stats.pendingTransitions, 0);
    EXPECT_FALSE(sceneManager->isTransitioning());
}

// ============================================================================
// Statistics and Diagnostics Tests
// ============================================================================

TEST_F(SceneManagerTest, Statistics_TrackTransitionsCorrectly) {
    auto initialStats = sceneManager->getStatistics();
    EXPECT_EQ(initialStats.totalTransitions, 0);
    EXPECT_EQ(initialStats.failedTransitions, 0);
    
    // Successful transitions
    sceneManager->requestSceneTransition("Scene1");
    sceneManager->processTransitions();
    
    sceneManager->requestSceneTransition("Scene2");
    sceneManager->processTransitions();
    
    // Failed transition
    sceneManager->requestSceneTransition("FailingScene");
    sceneManager->processTransitions();
    
    auto finalStats = sceneManager->getStatistics();
    EXPECT_EQ(finalStats.totalTransitions, 3);
    EXPECT_EQ(finalStats.failedTransitions, 1);
    EXPECT_EQ(finalStats.currentSceneName, "Scene2"); // Should rollback to Scene2
}

TEST_F(SceneManagerTest, IsValid_ReflectsManagerState) {
    EXPECT_TRUE(sceneManager->isValid()); // Initial state
    
    sceneManager->requestSceneTransition("Scene1");
    sceneManager->processTransitions();
    EXPECT_TRUE(sceneManager->isValid()); // Active scene
    
    sceneManager->requestSceneTransition("FailingScene");
    sceneManager->processTransitions();
    EXPECT_TRUE(sceneManager->isValid()); // Should still be valid (rolled back)
}

// ============================================================================
// Backward Compatibility Tests
// ============================================================================

TEST_F(SceneManagerTest, DeprecatedLoadScene_StillWorks) {
    // Test deprecated API still functions
    sceneManager->loadScene("Scene1");
    
    auto scene = getMockScene();
    ASSERT_NE(scene, nullptr);
    EXPECT_EQ(scene->getName(), "Scene1");
}

// ============================================================================
// Resource Lifecycle Tests
// ============================================================================

TEST_F(SceneManagerTest, SceneDestruction_CallsUnloadOnDestroy) {
    sceneManager->requestSceneTransition("Scene1");
    sceneManager->processTransitions();
    
    auto scene = getMockScene();
    ASSERT_NE(scene, nullptr);
    
    // Destroy scene manager - should unload current scene
    sceneManager.reset();
    
    // Scene should have been unloaded
    EXPECT_EQ(scene->getUnloadCalls(), 1);
}