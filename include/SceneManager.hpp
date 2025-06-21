#pragma once

#include "BaseScene.hpp"
#include <functional>
#include <iostream>
#include <memory>
#include <unordered_map>
#include <mutex>
#include <queue>
#include <string>

/**
 * Thread-safe SceneManager with state machine for robust scene transitions
 * 
 * Eliminates race conditions by:
 * - Deferred scene transitions (executed at safe points)
 * - Atomic scene updates (never leaves getCurrentScene() null)
 * - Exception safety with rollback capability
 * - Proper resource lifecycle management
 */

enum class SceneState {
    INACTIVE,    // Scene not loaded
    LOADING,     // Scene being loaded  
    ACTIVE,      // Scene ready for use
    UNLOADING,   // Scene being unloaded
    FAILED       // Scene load/unload failed
};

/**
 * Internal scene wrapper with state tracking
 */
struct SceneContext {
    std::shared_ptr<BaseScene> scene;
    SceneState state = SceneState::INACTIVE;
    std::string name;
    std::string errorMessage;
    
    SceneContext() = default;
    SceneContext(const std::string& sceneName) : name(sceneName) {}
};

/**
 * Scene transition request for deferred execution
 */
struct SceneTransition {
    std::string targetSceneName;
    bool force = false; // Force transition even if current scene is same
    
    SceneTransition(const std::string& name, bool forceReload = false) 
        : targetSceneName(name), force(forceReload) {}
};

class SceneManager {
private:
    // Scene factories and current state
    std::unordered_map<std::string, std::function<std::shared_ptr<BaseScene>()>> m_sceneFactories;
    SceneContext m_currentSceneContext;
    SceneContext m_previousSceneContext; // For rollback on failure
    
    // Thread safety
    mutable std::mutex m_mutex;
    
    // Deferred transition system
    std::queue<SceneTransition> m_pendingTransitions;
    bool m_isTransitioning = false;
    
    // Statistics and debugging
    size_t m_totalTransitions = 0;
    size_t m_failedTransitions = 0;
    
    /**
     * @brief Internal scene loading with exception safety
     * @param name Name of scene to load
     * @param force Force reload even if scene is already active
     * @return True if scene loaded successfully
     * 
     * Called from processTransitions() in thread-safe context.
     * Handles factory invocation, scene initialization, and error recovery.
     */
    bool loadSceneInternal(const std::string& name, bool force);
    
    /**
     * @brief Internal scene unloading with error handling
     * @param context Scene context to unload
     * @return True if unloaded successfully
     * 
     * Calls scene's onUnload() method and handles any exceptions.
     */
    bool unloadSceneInternal(SceneContext& context);
    
    /**
     * @brief Rollback to previous scene if new scene fails
     * 
     * Restores previous scene state when a scene transition fails.
     * Provides graceful degradation instead of leaving system in broken state.
     */
    void rollbackToPreviousScene();

public:
    SceneManager() = default;
    ~SceneManager();
    
    // ========================================================================
    // Scene Registration and Management
    // ========================================================================
    
    /**
     * Register a scene factory for later instantiation
     * Thread-safe
     */
    void registerScene(const std::string& name,
                      std::function<std::shared_ptr<BaseScene>()> factory);
    
    /**
     * Request a scene transition (deferred execution)
     * Thread-safe - queues the transition for safe execution
     * @param name Scene name to load
     * @param force Force reload even if scene is already active
     */
    void requestSceneTransition(const std::string& name, bool force = false);
    
    /**
     * Process pending scene transitions at safe points in game loop
     * Should be called once per frame before scene operations
     * NOT thread-safe - call only from main thread
     */
    void processTransitions();
    
    /**
     * Get current scene safely
     * Thread-safe - guaranteed to return valid scene or nullptr
     * Never returns a scene in LOADING/UNLOADING state
     */
    std::shared_ptr<BaseScene> getCurrentScene() const;
    
    /**
     * Check if a scene transition is currently in progress
     * Thread-safe
     */
    bool isTransitioning() const;
    
    /**
     * Get current scene name
     * Thread-safe
     */
    std::string getCurrentSceneName() const;
    
    /**
     * Get current scene state
     * Thread-safe
     */
    SceneState getCurrentSceneState() const;
    
    // ========================================================================
    // Error Handling and Diagnostics
    // ========================================================================
    
    /**
     * Check if scene manager is in valid state
     */
    bool isValid() const;
    
    /**
     * Get last error message
     */
    std::string getLastError() const;
    
    /**
     * Get transition statistics
     */
    struct Statistics {
        size_t totalTransitions = 0;
        size_t failedTransitions = 0;
        size_t pendingTransitions = 0;
        bool isTransitioning = false;
        std::string currentSceneName;
        SceneState currentSceneState = SceneState::INACTIVE;
    };
    
    Statistics getStatistics() const;
    
    /**
     * Clear all pending transitions (emergency use)
     */
    void clearPendingTransitions();
    
    // ========================================================================
    // Backward Compatibility (deprecated)
    // ========================================================================
    
    /**
     * @deprecated Use requestSceneTransition() instead
     * Immediate scene loading - unsafe for concurrent use
     * Kept for backward compatibility but not recommended
     */
    [[deprecated("Use requestSceneTransition() for thread safety")]]
    void loadScene(const std::string& name);
};