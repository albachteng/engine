#include "../include/SceneManager.hpp"
#include "../include/Logger.hpp"
#include <stdexcept>
#include <chrono>

SceneManager::~SceneManager() {
    std::lock_guard<std::mutex> lock(m_mutex);
    
    // Unload current scene safely
    if (m_currentSceneContext.scene && m_currentSceneContext.state == SceneState::ACTIVE) {
        m_currentSceneContext.state = SceneState::UNLOADING;
        try {
            m_currentSceneContext.scene->onUnload();
            LOG_INFO_STREAM("SceneManager: Successfully unloaded scene '" 
                      << m_currentSceneContext.name << "' during shutdown");
        } catch (const std::exception& e) {
            LOG_ERROR_STREAM("SceneManager: Error unloading scene during shutdown: " 
                      << e.what());
        }
    }
    
    // Clear pending transitions
    while (!m_pendingTransitions.empty()) {
        m_pendingTransitions.pop();
    }
}

void SceneManager::registerScene(const std::string& name,
                                std::function<std::shared_ptr<BaseScene>()> factory) {
    std::lock_guard<std::mutex> lock(m_mutex);
    
    if (name.empty()) {
        throw std::invalid_argument("Scene name cannot be empty");
    }
    
    if (!factory) {
        throw std::invalid_argument("Scene factory cannot be null");
    }
    
    m_sceneFactories[name] = factory;
    LOG_INFO_STREAM("SceneManager: Registered scene factory '" << name << "'");
}

void SceneManager::requestSceneTransition(const std::string& name, bool force) {
    std::lock_guard<std::mutex> lock(m_mutex);
    
    if (name.empty()) {
        LOG_ERROR("SceneManager: Cannot transition to empty scene name");
        return;
    }
    
    if (m_sceneFactories.find(name) == m_sceneFactories.end()) {
        LOG_ERROR_STREAM("SceneManager: Scene '" << name << "' not registered");
        return;
    }
    
    // Skip duplicate transitions unless forced
    if (!force && m_currentSceneContext.name == name && 
        m_currentSceneContext.state == SceneState::ACTIVE) {
        LOG_INFO_STREAM("SceneManager: Scene '" << name << "' already active, skipping transition");
        return;
    }
    
    // Add to pending transitions
    m_pendingTransitions.emplace(name, force);
    LOG_INFO_STREAM("SceneManager: Queued transition to scene '" << name << "'");
}

void SceneManager::processTransitions() {
    // NOTE: This method is NOT thread-safe by design
    // It should only be called from the main game thread
    
    if (m_isTransitioning) {
        return; // Already processing a transition
    }
    
    std::unique_lock<std::mutex> lock(m_mutex);
    
    if (m_pendingTransitions.empty()) {
        return; // No pending transitions
    }
    
    // Get next transition
    SceneTransition transition = m_pendingTransitions.front();
    m_pendingTransitions.pop();
    
    m_isTransitioning = true;
    lock.unlock(); // Release lock during actual scene operations
    
    LOG_INFO_STREAM("SceneManager: Processing transition to '" << transition.targetSceneName << "'");
    
    bool success = loadSceneInternal(transition.targetSceneName, transition.force);
    
    lock.lock();
    m_isTransitioning = false;
    m_totalTransitions++;
    
    if (!success) {
        m_failedTransitions++;
        LOG_ERROR_STREAM("SceneManager: Failed to transition to '" << transition.targetSceneName << "'");
    } else {
        LOG_INFO_STREAM("SceneManager: Successfully transitioned to '" << transition.targetSceneName << "'");
    }
}

bool SceneManager::loadSceneInternal(const std::string& name, bool force) {
    auto start = std::chrono::high_resolution_clock::now();
    
    // Backup current scene for potential rollback
    m_previousSceneContext = m_currentSceneContext;
    
    try {
        // Step 1: Unload current scene if exists
        if (m_currentSceneContext.scene && m_currentSceneContext.state == SceneState::ACTIVE) {
            LOG_INFO_STREAM("SceneManager: Unloading current scene '" << m_currentSceneContext.name << "'");
            
            if (!unloadSceneInternal(m_currentSceneContext)) {
                LOG_ERROR("SceneManager: Failed to unload current scene, aborting transition");
                return false;
            }
        }
        
        // Step 2: Create new scene context
        SceneContext newContext(name);
        newContext.state = SceneState::LOADING;
        
        LOG_INFO_STREAM("SceneManager: Loading scene '" << name << "'");
        
        // Step 3: Create scene instance
        auto factory = m_sceneFactories[name];
        newContext.scene = factory();
        
        if (!newContext.scene) {
            newContext.state = SceneState::FAILED;
            newContext.errorMessage = "Scene factory returned null";
            throw std::runtime_error("Scene factory returned null for '" + name + "'");
        }
        
        // Step 4: Initialize scene
        newContext.scene->onLoad();
        newContext.state = SceneState::ACTIVE;
        
        // Step 5: Atomically update current scene
        {
            std::lock_guard<std::mutex> lock(m_mutex);
            m_currentSceneContext = newContext;
        }
        
        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration<double, std::milli>(end - start).count();
        
        LOG_INFO_STREAM("SceneManager: Scene '" << name << "' loaded successfully in " 
                  << duration << "ms");
        
        return true;
        
    } catch (const std::exception& e) {
        LOG_ERROR_STREAM("SceneManager: Exception during scene loading: " << e.what());
        
        // Attempt rollback to previous scene
        rollbackToPreviousScene();
        return false;
    }
}

bool SceneManager::unloadSceneInternal(SceneContext& context) {
    if (!context.scene || context.state != SceneState::ACTIVE) {
        return true; // Nothing to unload
    }
    
    try {
        context.state = SceneState::UNLOADING;
        context.scene->onUnload();
        context.scene.reset();
        context.state = SceneState::INACTIVE;
        
        LOG_INFO_STREAM("SceneManager: Scene '" << context.name << "' unloaded successfully");
        return true;
        
    } catch (const std::exception& e) {
        context.state = SceneState::FAILED;
        context.errorMessage = e.what();
        LOG_ERROR_STREAM("SceneManager: Error unloading scene '" << context.name << "': " << e.what());
        return false;
    }
}

void SceneManager::rollbackToPreviousScene() {
    LOG_INFO("SceneManager: Attempting rollback to previous scene");
    
    if (!m_previousSceneContext.scene || m_previousSceneContext.state != SceneState::ACTIVE) {
        LOG_WARN("SceneManager: No valid previous scene for rollback");
        
        // Create empty scene context as fallback
        std::lock_guard<std::mutex> lock(m_mutex);
        m_currentSceneContext = SceneContext();
        m_currentSceneContext.state = SceneState::FAILED;
        m_currentSceneContext.errorMessage = "Scene load failed and no valid previous scene";
        return;
    }
    
    try {
        // Restore previous scene
        {
            std::lock_guard<std::mutex> lock(m_mutex);
            m_currentSceneContext = m_previousSceneContext;
        }
        
        LOG_INFO_STREAM("SceneManager: Successfully rolled back to scene '" 
                  << m_previousSceneContext.name << "'");
        
    } catch (const std::exception& e) {
        LOG_ERROR_STREAM("SceneManager: Rollback failed: " << e.what());
        
        // Complete failure - clear current scene
        std::lock_guard<std::mutex> lock(m_mutex);
        m_currentSceneContext = SceneContext();
        m_currentSceneContext.state = SceneState::FAILED;
        m_currentSceneContext.errorMessage = "Scene load and rollback both failed";
    }
}

std::shared_ptr<BaseScene> SceneManager::getCurrentScene() const {
    std::lock_guard<std::mutex> lock(m_mutex);
    
    // Only return scene if it's in ACTIVE state
    if (m_currentSceneContext.state == SceneState::ACTIVE && m_currentSceneContext.scene) {
        return m_currentSceneContext.scene;
    }
    
    return nullptr;
}

bool SceneManager::isTransitioning() const {
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_isTransitioning || !m_pendingTransitions.empty();
}

std::string SceneManager::getCurrentSceneName() const {
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_currentSceneContext.name;
}

SceneState SceneManager::getCurrentSceneState() const {
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_currentSceneContext.state;
}

bool SceneManager::isValid() const {
    std::lock_guard<std::mutex> lock(m_mutex);
    
    // Valid if we have an active scene or no scene at all (initial state)
    return m_currentSceneContext.state == SceneState::ACTIVE || 
           m_currentSceneContext.state == SceneState::INACTIVE;
}

std::string SceneManager::getLastError() const {
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_currentSceneContext.errorMessage;
}

SceneManager::Statistics SceneManager::getStatistics() const {
    std::lock_guard<std::mutex> lock(m_mutex);
    
    Statistics stats;
    stats.totalTransitions = m_totalTransitions;
    stats.failedTransitions = m_failedTransitions;
    stats.pendingTransitions = m_pendingTransitions.size();
    stats.isTransitioning = m_isTransitioning;
    stats.currentSceneName = m_currentSceneContext.name;
    stats.currentSceneState = m_currentSceneContext.state;
    
    return stats;
}

void SceneManager::clearPendingTransitions() {
    std::lock_guard<std::mutex> lock(m_mutex);
    
    size_t cleared = m_pendingTransitions.size();
    while (!m_pendingTransitions.empty()) {
        m_pendingTransitions.pop();
    }
    
    if (cleared > 0) {
        LOG_INFO_STREAM("SceneManager: Cleared " << cleared << " pending transitions");
    }
}

// Backward compatibility - deprecated
void SceneManager::loadScene(const std::string& name) {
    LOG_WARN("SceneManager: WARNING - loadScene() is deprecated and unsafe. "
              "Use requestSceneTransition() instead.");
    
    // For backward compatibility, queue the transition
    requestSceneTransition(name, false);
    
    // Process immediately (unsafe but maintains old behavior)
    processTransitions();
}