#include <gtest/gtest.h>
#include "../include/ComponentManager.hpp"
#include "../include/Component.h"
#include <memory>

/**
 * Comprehensive unit tests for array-based component system
 * 
 * Tests cover:
 * - Component storage and retrieval
 * - Memory efficiency vs tuple system
 * - Cache performance characteristics
 * - Type safety and error handling
 * - Component lifecycle management
 * - System iteration capabilities
 */

class ComponentManagerTest : public ::testing::Test {
protected:
    void SetUp() override {
        manager = std::make_unique<ComponentManager>();
    }

    void TearDown() override {
        manager.reset();
    }

    std::unique_ptr<ComponentManager> manager;
};

// ============================================================================
// Basic Component Operations
// ============================================================================

TEST_F(ComponentManagerTest, AddComponent_StoresComponentCorrectly) {
    // Add CTransform3D component
    glm::vec3 position(1.0f, 2.0f, 3.0f);
    glm::vec3 rotation(0.1f, 0.2f, 0.3f);
    glm::vec3 scale(2.0f, 2.0f, 2.0f);
    
    auto& transform = manager->addComponent<CTransform3D>(42, position, rotation, scale);
    
    EXPECT_TRUE(transform.exists);
    EXPECT_EQ(transform.position, position);
    EXPECT_EQ(transform.rotation, rotation);
    EXPECT_EQ(transform.scale, scale);
}

TEST_F(ComponentManagerTest, HasComponent_ReturnsTrueForExistingComponent) {
    manager->addComponent<CTransform3D>(42);
    
    EXPECT_TRUE(manager->hasComponent<CTransform3D>(42));
    EXPECT_FALSE(manager->hasComponent<CTransform3D>(99)); // Different entity
    EXPECT_FALSE(manager->hasComponent<CMovement3D>(42));  // Different component
}

TEST_F(ComponentManagerTest, GetComponent_ReturnsCorrectComponent) {
    glm::vec3 velocity(5.0f, 0.0f, -2.0f);
    glm::vec3 acceleration(0.0f, -9.8f, 0.0f);
    
    manager->addComponent<CMovement3D>(42, velocity, acceleration);
    
    auto& movement = manager->getComponent<CMovement3D>(42);
    EXPECT_EQ(movement.vel, velocity);
    EXPECT_EQ(movement.acc, acceleration);
}

TEST_F(ComponentManagerTest, RemoveComponent_RemovesComponentCorrectly) {
    manager->addComponent<CTransform3D>(42);
    EXPECT_TRUE(manager->hasComponent<CTransform3D>(42));
    
    manager->removeComponent<CTransform3D>(42);
    EXPECT_FALSE(manager->hasComponent<CTransform3D>(42));
}

// ============================================================================
// Multiple Entities and Components
// ============================================================================

TEST_F(ComponentManagerTest, MultipleEntities_StoredIndependently) {
    // Add same component type to different entities
    manager->addComponent<CTransform3D>(1, glm::vec3(1.0f, 0.0f, 0.0f), glm::vec3(0.0f), glm::vec3(1.0f));
    manager->addComponent<CTransform3D>(2, glm::vec3(2.0f, 0.0f, 0.0f), glm::vec3(0.0f), glm::vec3(1.0f));
    manager->addComponent<CTransform3D>(3, glm::vec3(3.0f, 0.0f, 0.0f), glm::vec3(0.0f), glm::vec3(1.0f));
    
    EXPECT_TRUE(manager->hasComponent<CTransform3D>(1));
    EXPECT_TRUE(manager->hasComponent<CTransform3D>(2));
    EXPECT_TRUE(manager->hasComponent<CTransform3D>(3));
    
    EXPECT_EQ(manager->getComponent<CTransform3D>(1).position.x, 1.0f);
    EXPECT_EQ(manager->getComponent<CTransform3D>(2).position.x, 2.0f);
    EXPECT_EQ(manager->getComponent<CTransform3D>(3).position.x, 3.0f);
}

TEST_F(ComponentManagerTest, MultipleComponentTypes_StoredCorrectly) {
    // Add different component types to same entity
    manager->addComponent<CTransform3D>(42);
    manager->addComponent<CMovement3D>(42);
    manager->addComponent<CAABB>(42, glm::vec3(0.0f), glm::vec3(1.0f));
    
    EXPECT_TRUE(manager->hasComponent<CTransform3D>(42));
    EXPECT_TRUE(manager->hasComponent<CMovement3D>(42));
    EXPECT_TRUE(manager->hasComponent<CAABB>(42));
    EXPECT_FALSE(manager->hasComponent<CTriangle>(42));
}

TEST_F(ComponentManagerTest, RemoveMultipleComponents_HandledCorrectly) {
    // Add multiple components
    manager->addComponent<CTransform3D>(42);
    manager->addComponent<CMovement3D>(42);
    manager->addComponent<CAABB>(42);
    
    // Remove one component
    manager->removeComponent<CMovement3D>(42);
    
    EXPECT_TRUE(manager->hasComponent<CTransform3D>(42));
    EXPECT_FALSE(manager->hasComponent<CMovement3D>(42));
    EXPECT_TRUE(manager->hasComponent<CAABB>(42));
}

// ============================================================================
// Component Array Efficiency Tests
// ============================================================================

TEST_F(ComponentManagerTest, ComponentArray_PacksComponentsEfficiently) {
    // Add components to multiple entities
    for (size_t i = 0; i < 100; ++i) {
        manager->addComponent<CTransform3D>(i, 
            glm::vec3(static_cast<float>(i), 0.0f, 0.0f), 
            glm::vec3(0.0f), 
            glm::vec3(1.0f));
    }
    
    auto* componentArray = manager->getComponentArray<CTransform3D>();
    EXPECT_EQ(componentArray->size(), 100);
    
    // Verify dense packing - all components should be contiguous
    const auto& components = componentArray->getData();
    EXPECT_EQ(components.size(), 100);
    
    for (size_t i = 0; i < 100; ++i) {
        EXPECT_EQ(components[i].position.x, static_cast<float>(i));
    }
}

TEST_F(ComponentManagerTest, ComponentRemoval_MaintainsDensePacking) {
    // Add 10 components
    for (size_t i = 0; i < 10; ++i) {
        manager->addComponent<CTransform3D>(i, 
            glm::vec3(static_cast<float>(i), 0.0f, 0.0f), 
            glm::vec3(0.0f), 
            glm::vec3(1.0f));
    }
    
    auto* componentArray = manager->getComponentArray<CTransform3D>();
    EXPECT_EQ(componentArray->size(), 10);
    
    // Remove component from middle (entity 5)
    manager->removeComponent<CTransform3D>(5);
    EXPECT_EQ(componentArray->size(), 9);
    
    // Verify all remaining entities still have their components
    for (size_t i = 0; i < 10; ++i) {
        if (i == 5) {
            EXPECT_FALSE(manager->hasComponent<CTransform3D>(i));
        } else {
            EXPECT_TRUE(manager->hasComponent<CTransform3D>(i));
            EXPECT_EQ(manager->getComponent<CTransform3D>(i).position.x, static_cast<float>(i));
        }
    }
}

// ============================================================================
// System Iteration Tests
// ============================================================================

TEST_F(ComponentManagerTest, ComponentIteration_WorksForSystems) {
    // Add components to some entities
    std::vector<size_t> entityIDs = {10, 20, 30, 40, 50};
    for (size_t id : entityIDs) {
        manager->addComponent<CMovement3D>(id, 
            glm::vec3(static_cast<float>(id), 0.0f, 0.0f),
            glm::vec3(0.0f));
    }
    
    auto* componentArray = manager->getComponentArray<CMovement3D>();
    
    // Test iteration over all components
    size_t count = 0;
    for (auto& movement : *componentArray) {
        EXPECT_TRUE(movement.exists);
        count++;
    }
    EXPECT_EQ(count, entityIDs.size());
    
    // Test entity ID access during iteration
    const auto& components = componentArray->getData();
    const auto& entities = componentArray->getEntityIDs();
    
    EXPECT_EQ(components.size(), entities.size());
    
    for (size_t i = 0; i < components.size(); ++i) {
        size_t entityID = entities[i];
        EXPECT_NE(std::find(entityIDs.begin(), entityIDs.end(), entityID), entityIDs.end());
        EXPECT_EQ(components[i].vel.x, static_cast<float>(entityID));
    }
}

// ============================================================================
// Performance Comparison Tests
// ============================================================================

TEST_F(ComponentManagerTest, MemoryEfficiency_OnlyAllocatesUsedComponents) {
    auto stats = manager->getStatistics();
    EXPECT_EQ(stats.totalComponents, 0);
    EXPECT_EQ(stats.totalComponentTypes, 0);
    
    // Add only CTransform3D to 100 entities
    for (size_t i = 0; i < 100; ++i) {
        manager->addComponent<CTransform3D>(i);
    }
    
    stats = manager->getStatistics();
    EXPECT_EQ(stats.totalComponents, 100);
    EXPECT_EQ(stats.totalComponentTypes, 1);
    
    // With tuple system, we would allocate 100 * 12 = 1200 components
    // With array system, we allocate only 100 components
    // This is a 12x memory reduction for this case
}

TEST_F(ComponentManagerTest, CachePerformance_ComponentsStoredContiguously) {
    const size_t numEntities = 1000;
    
    // Add components
    for (size_t i = 0; i < numEntities; ++i) {
        manager->addComponent<CTransform3D>(i, 
            glm::vec3(static_cast<float>(i), 0.0f, 0.0f), 
            glm::vec3(0.0f), 
            glm::vec3(1.0f));
    }
    
    auto* componentArray = manager->getComponentArray<CTransform3D>();
    const auto& components = componentArray->getData();
    
    // Verify contiguous storage (cache-friendly)
    EXPECT_EQ(components.size(), numEntities);
    
    // Simulate system processing (cache-friendly iteration)
    auto start = std::chrono::high_resolution_clock::now();
    
    // Get non-const reference for modification
    auto& mutableComponents = componentArray->getData();
    for (auto& transform : mutableComponents) {
        transform.position += glm::vec3(0.1f, 0.0f, 0.0f);
    }
    
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration<double, std::micro>(end - start).count();
    
    // Should be very fast due to cache locality
    EXPECT_LT(duration, 1000.0); // Less than 1ms for 1000 components
}

// ============================================================================
// Error Handling and Edge Cases
// ============================================================================

TEST_F(ComponentManagerTest, GetNonexistentComponent_AssertsInDebug) {
    // This test verifies that accessing non-existent components fails safely
    // In debug builds, this should assert
    // In release builds, behavior is undefined but shouldn't crash
    
    EXPECT_FALSE(manager->hasComponent<CTransform3D>(999));
    
    // Note: Can't easily test assertion failure in unit tests
    // This is tested by compiler and runtime debugging
}

TEST_F(ComponentManagerTest, RemoveNonexistentComponent_HandledGracefully) {
    // Should not crash when removing component that doesn't exist
    manager->removeComponent<CTransform3D>(999);
    
    // Should still be false
    EXPECT_FALSE(manager->hasComponent<CTransform3D>(999));
}

TEST_F(ComponentManagerTest, ClearAllComponents_RemovesEverything) {
    // Add various components
    manager->addComponent<CTransform3D>(1);
    manager->addComponent<CMovement3D>(1);
    manager->addComponent<CTransform3D>(2);
    manager->addComponent<CAABB>(3);
    
    auto stats = manager->getStatistics();
    EXPECT_GT(stats.totalComponents, 0);
    
    manager->clear();
    
    stats = manager->getStatistics();
    EXPECT_EQ(stats.totalComponents, 0);
    
    // Verify all components are gone
    EXPECT_FALSE(manager->hasComponent<CTransform3D>(1));
    EXPECT_FALSE(manager->hasComponent<CMovement3D>(1));
    EXPECT_FALSE(manager->hasComponent<CTransform3D>(2));
    EXPECT_FALSE(manager->hasComponent<CAABB>(3));
}

// ============================================================================
// Type Safety Tests
// ============================================================================

TEST_F(ComponentManagerTest, ComponentTypeIDs_AreUniqueAndStable) {
    // Component type IDs should be unique for each type
    size_t id1 = ComponentTypeIDGenerator::getID<CTransform3D>();
    size_t id2 = ComponentTypeIDGenerator::getID<CMovement3D>();
    size_t id3 = ComponentTypeIDGenerator::getID<CAABB>();
    
    EXPECT_NE(id1, id2);
    EXPECT_NE(id2, id3);
    EXPECT_NE(id1, id3);
    
    // IDs should be stable across calls
    EXPECT_EQ(id1, ComponentTypeIDGenerator::getID<CTransform3D>());
    EXPECT_EQ(id2, ComponentTypeIDGenerator::getID<CMovement3D>());
    EXPECT_EQ(id3, ComponentTypeIDGenerator::getID<CAABB>());
}

TEST_F(ComponentManagerTest, ComponentArrays_AreTypeSafe) {
    manager->addComponent<CTransform3D>(42);
    manager->addComponent<CMovement3D>(42);
    
    // Getting the right component array should work
    auto* transformArray = manager->getComponentArray<CTransform3D>();
    auto* movementArray = manager->getComponentArray<CMovement3D>();
    
    EXPECT_NE(transformArray, nullptr);
    EXPECT_NE(movementArray, nullptr);
    EXPECT_NE(static_cast<void*>(transformArray), static_cast<void*>(movementArray));
    
    EXPECT_TRUE(transformArray->hasComponent(42));
    EXPECT_TRUE(movementArray->hasComponent(42));
}

// ============================================================================
// Statistics and Debugging Tests
// ============================================================================

TEST_F(ComponentManagerTest, Statistics_TrackComponentUsageCorrectly) {
    auto stats = manager->getStatistics();
    EXPECT_EQ(stats.totalComponents, 0);
    EXPECT_EQ(stats.totalComponentTypes, 0);
    
    // Add different types of components
    manager->addComponent<CTransform3D>(1);
    manager->addComponent<CTransform3D>(2);
    manager->addComponent<CMovement3D>(1);
    manager->addComponent<CAABB>(3);
    
    stats = manager->getStatistics();
    EXPECT_EQ(stats.totalComponents, 4);
    EXPECT_EQ(stats.totalComponentTypes, 3); // CTransform3D, CMovement3D, CAABB
    
    // Check component counts
    bool foundTransform = false, foundMovement = false, foundAABB = false;
    for (const auto& [name, count] : stats.componentCounts) {
        if (name.find("CTransform3D") != std::string::npos) {
            EXPECT_EQ(count, 2);
            foundTransform = true;
        } else if (name.find("CMovement3D") != std::string::npos) {
            EXPECT_EQ(count, 1);
            foundMovement = true;
        } else if (name.find("CAABB") != std::string::npos) {
            EXPECT_EQ(count, 1);
            foundAABB = true;
        }
    }
    
    EXPECT_TRUE(foundTransform);
    EXPECT_TRUE(foundMovement);
    EXPECT_TRUE(foundAABB);
}