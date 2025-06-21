#pragma once
#include "Component.h"
#include "ComponentManager.hpp"
#include <bitset>
#include <memory>

enum class EntityTag {
  DEFAULT,
  TRIANGLE,
  PLAYER,
  MAP_NODE,
  ENEMY
};

/**
 * Array-based Entity implementation
 * 
 * Replaces tuple-based storage with efficient component arrays:
 * - Same API as original Entity class (drop-in replacement)
 * - Memory efficient: Only allocates used components
 * - Cache friendly: Components stored contiguously
 * - Type safe: Compile-time component registration
 * - System friendly: Enables efficient component iteration
 */
class Entity {
private:
    // Entity identification
    size_t m_id = 0;
    EntityTag m_tag = EntityTag::DEFAULT;
    bool m_active = true;
    
    // Component tracking
    std::bitset<MAX_COMPONENTS> m_componentMask;
    
    // Shared component manager (static for all entities)
    static std::unique_ptr<ComponentManager> s_componentManager;
    
    // Private constructor for EntityManager
    Entity(const size_t &id, const EntityTag &tag) : m_id(id), m_tag(tag) {}
    
    friend class EntityManager;
    
public:
    /**
     * Initialize component manager (called once at startup)
     */
    static void initializeComponentManager() {
        if (!s_componentManager) {
            s_componentManager = std::make_unique<ComponentManager>();
        }
    }
    
    /**
     * Get component manager (for testing/debugging)
     */
    static ComponentManager* getComponentManager() {
        return s_componentManager.get();
    }
    
    /**
     * Clean up component manager (called at shutdown)
     */
    static void shutdownComponentManager() {
        s_componentManager.reset();
    }
    
    // ========================================================================
    // Core Entity Interface (unchanged from original)
    // ========================================================================
    
    size_t id() const;
    bool isActive() const;
    void destroy();
    const EntityTag &tag() const;
    
    // ========================================================================
    // Component Interface (same API, array-based implementation)
    // ========================================================================
    
    /**
     * Get component reference
     * Same API as tuple-based system: entity->get<CTransform>()
     */
    template <typename T> 
    T &get() { 
        assert(s_componentManager && "Component manager not initialized");
        assert(has<T>() && "Entity does not have requested component");
        return s_componentManager->getComponent<T>(m_id);
    }
    
    template <typename T> 
    const T &get() const { 
        assert(s_componentManager && "Component manager not initialized");
        assert(has<T>() && "Entity does not have requested component");
        return s_componentManager->getComponent<T>(m_id);
    }
    
    /**
     * Check if entity has component
     * Same API as tuple-based system: entity->has<CTransform>()
     */
    template <typename T> 
    bool has() const { 
        if (!s_componentManager) return false;
        
        size_t componentID = ComponentTypeIDGenerator::getID<T>();
        return componentID < MAX_COMPONENTS && 
               m_componentMask[componentID] && 
               s_componentManager->hasComponent<T>(m_id);
    }
    
    /**
     * Add component to entity
     * Same API as tuple-based system: entity->add<CTransform>(args...)
     */
    template <typename T, typename... TArgs> 
    T &add(TArgs &&...mArgs) {
        assert(s_componentManager && "Component manager not initialized");
        
        size_t componentID = ComponentTypeIDGenerator::getID<T>();
        assert(componentID < MAX_COMPONENTS && "Too many component types");
        
        // Remove existing component if present
        if (has<T>()) {
            remove<T>();
        }
        
        // Add new component
        T& component = s_componentManager->addComponent<T>(m_id, std::forward<TArgs>(mArgs)...);
        m_componentMask[componentID] = true;
        
        return component;
    }
    
    /**
     * Remove component from entity
     * Same API as tuple-based system: entity->remove<CTransform>()
     */
    template <typename T> 
    void remove() { 
        if (!s_componentManager || !has<T>()) {
            return;
        }
        
        size_t componentID = ComponentTypeIDGenerator::getID<T>();
        s_componentManager->removeComponent<T>(m_id);
        m_componentMask[componentID] = false;
    }
    
    // ========================================================================
    // Advanced Component Interface (new capabilities)
    // ========================================================================
    
    /**
     * Get component mask for efficient system queries
     */
    const std::bitset<MAX_COMPONENTS>& getComponentMask() const {
        return m_componentMask;
    }
    
    /**
     * Check if entity matches component signature
     * Useful for ECS systems that need entities with specific components
     */
    template<typename... Components>
    bool hasComponents() const {
        return (has<Components>() && ...);
    }
    
    /**
     * Get multiple components at once
     * Returns tuple of component references
     */
    template<typename... Components>
    std::tuple<Components&...> getComponents() {
        return std::tie(get<Components>()...);
    }
    
    /**
     * Remove all components from entity
     * Called automatically when entity is destroyed
     */
    void removeAllComponents() {
        if (s_componentManager) {
            s_componentManager->removeAllComponents(m_id);
            m_componentMask.reset();
        }
    }
    
    /**
     * Get component count for this entity
     */
    size_t getComponentCount() const {
        return m_componentMask.count();
    }
    
    /**
     * Check if entity has any components
     */
    bool hasAnyComponents() const {
        return m_componentMask.any();
    }
    
    /**
     * Destructor - clean up components when entity is destroyed
     */
    ~Entity() {
        if (s_componentManager) {
            removeAllComponents();
        }
    }
};
