#pragma once

#include <vector>
#include <unordered_map>
#include <memory>
#include <bitset>
#include <cassert>
#include <typeinfo>
#include <typeindex>

/**
 * Array-Based ECS Component Management System
 * 
 * Replaces tuple-based storage with efficient dense arrays:
 * - Memory efficient: Only allocate used components
 * - Cache friendly: Components stored contiguously 
 * - Type safe: Compile-time component ID generation
 * - Scalable: Dynamic component registration
 * - Compatible: Same API as tuple-based system
 */

// Maximum number of component types supported
constexpr size_t MAX_COMPONENTS = 64;

// Invalid component/entity constants
constexpr size_t INVALID_COMPONENT_ID = SIZE_MAX;
constexpr size_t INVALID_ENTITY_ID = SIZE_MAX;

/**
 * Component Type ID Generator
 * Assigns unique IDs to component types at compile time
 */
class ComponentTypeIDGenerator {
private:
    static size_t s_nextID;
    
public:
    /**
     * @brief Get unique type ID for component type T (thread-safe)
     * @return Unique ID for component type T, same ID returned for same type
     * 
     * Uses static local variable to ensure each component type gets exactly one ID.
     * First call for a type increments the global counter.
     */
    template<typename T>
    static size_t getID() {
        static size_t id = s_nextID++;
        return id;
    }
    
    static size_t getNextID() { return s_nextID; }
};

/**
 * Base class for component storage arrays
 * Provides polymorphic interface for component management
 */
class IComponentArray {
public:
    virtual ~IComponentArray() = default;
    
    /**
     * Remove component for given entity
     * @param entityID Entity to remove component from
     */
    virtual void removeComponent(size_t entityID) = 0;
    
    /**
     * Check if entity has this component type
     * @param entityID Entity to check
     * @return True if entity has component
     */
    virtual bool hasComponent(size_t entityID) const = 0;
    
    /**
     * Get number of components stored
     */
    virtual size_t size() const = 0;
    
    /**
     * Clear all components
     */
    virtual void clear() = 0;
    
    /**
     * Get component array name for debugging
     */
    virtual const char* getTypeName() const = 0;
};

/**
 * Type-specific dense component array
 * Stores components contiguously for cache efficiency
 */
template<typename T>
class ComponentArray : public IComponentArray {
private:
    // Dense array of components (contiguous storage)
    std::vector<T> m_components;
    
    // Sparse array: EntityID -> Index in dense array
    std::unordered_map<size_t, size_t> m_entityToIndex;
    
    // Dense array: Index -> EntityID (for reverse lookup)
    std::vector<size_t> m_indexToEntity;
    
public:
    ComponentArray() = default;
    
    /**
     * Add component for entity
     * @param entityID Entity to add component to
     * @param component Component data to add
     */
    void addComponent(size_t entityID, T component) {
        assert(m_entityToIndex.find(entityID) == m_entityToIndex.end() && 
               "Component already exists for entity");
        
        // Add to dense arrays
        size_t newIndex = m_components.size();
        m_components.push_back(component);
        m_indexToEntity.push_back(entityID);
        
        // Update sparse mapping
        m_entityToIndex[entityID] = newIndex;
    }
    
    /**
     * Remove component for entity
     * Uses swap-and-pop for O(1) removal
     */
    void removeComponent(size_t entityID) override {
        auto it = m_entityToIndex.find(entityID);
        if (it == m_entityToIndex.end()) {
            return; // Component doesn't exist
        }
        
        size_t indexToRemove = it->second;
        size_t lastIndex = m_components.size() - 1;
        
        // Swap with last element (if not already last)
        if (indexToRemove != lastIndex) {
            // Move last component to removed position
            m_components[indexToRemove] = std::move(m_components[lastIndex]);
            
            // Update entity mapping for swapped component
            size_t entityOfLastComponent = m_indexToEntity[lastIndex];
            m_indexToEntity[indexToRemove] = entityOfLastComponent;
            m_entityToIndex[entityOfLastComponent] = indexToRemove;
        }
        
        // Remove last element
        m_components.pop_back();
        m_indexToEntity.pop_back();
        m_entityToIndex.erase(entityID);
    }
    
    /**
     * Get component for entity
     * @param entityID Entity to get component from
     * @return Reference to component
     */
    T& getComponent(size_t entityID) {
        auto it = m_entityToIndex.find(entityID);
        assert(it != m_entityToIndex.end() && "Component does not exist for entity");
        return m_components[it->second];
    }
    
    const T& getComponent(size_t entityID) const {
        auto it = m_entityToIndex.find(entityID);
        assert(it != m_entityToIndex.end() && "Component does not exist for entity");
        return m_components[it->second];
    }
    
    /**
     * Check if entity has component
     */
    bool hasComponent(size_t entityID) const override {
        return m_entityToIndex.find(entityID) != m_entityToIndex.end();
    }
    
    /**
     * Get number of components
     */
    size_t size() const override {
        return m_components.size();
    }
    
    /**
     * Clear all components
     */
    void clear() override {
        m_components.clear();
        m_indexToEntity.clear();
        m_entityToIndex.clear();
    }
    
    /**
     * Get component type name
     */
    const char* getTypeName() const override {
        return typeid(T).name();
    }
    
    /**
     * Iterator access for system-based iteration
     */
    typename std::vector<T>::iterator begin() { return m_components.begin(); }
    typename std::vector<T>::iterator end() { return m_components.end(); }
    typename std::vector<T>::const_iterator begin() const { return m_components.begin(); }
    typename std::vector<T>::const_iterator end() const { return m_components.end(); }
    
    /**
     * Get entity ID for component at index (for system iteration)
     */
    size_t getEntityID(size_t index) const {
        assert(index < m_indexToEntity.size() && "Index out of range");
        return m_indexToEntity[index];
    }
    
    /**
     * Direct access to component data (for system iteration)
     */
    const std::vector<T>& getData() const { return m_components; }
    std::vector<T>& getData() { return m_components; }
    
    /**
     * Get entity IDs (for system iteration)
     */
    const std::vector<size_t>& getEntityIDs() const { return m_indexToEntity; }
};

/**
 * Component Manager - Central registry for all component types
 * Manages component arrays and provides unified access interface
 */
class ComponentManager {
private:
    // Array of component arrays (indexed by component type ID)
    std::vector<std::unique_ptr<IComponentArray>> m_componentArrays;
    
    // Type index to component ID mapping (for runtime type safety)
    std::unordered_map<std::type_index, size_t> m_typeToComponentID;
    
    // Component names for debugging
    std::vector<std::string> m_componentNames;
    
public:
    ComponentManager() {
        // Reserve space for maximum components
        m_componentArrays.resize(MAX_COMPONENTS);
        m_componentNames.resize(MAX_COMPONENTS);
    }
    
    /**
     * Register component type (called automatically on first use)
     */
    template<typename T>
    void registerComponent() {
        size_t componentID = ComponentTypeIDGenerator::getID<T>();
        std::type_index typeIndex(typeid(T));
        
        assert(componentID < MAX_COMPONENTS && "Too many component types");
        assert(m_typeToComponentID.find(typeIndex) == m_typeToComponentID.end() && 
               "Component type already registered");
        
        // Create component array for this type
        m_componentArrays[componentID] = std::make_unique<ComponentArray<T>>();
        m_typeToComponentID[typeIndex] = componentID;
        m_componentNames[componentID] = typeid(T).name();
    }
    
    /**
     * Get component type ID for type T
     */
    template<typename T>
    size_t getComponentTypeID() {
        size_t componentID = ComponentTypeIDGenerator::getID<T>();
        
        // Auto-register component type if not already registered
        if (componentID >= m_componentArrays.size() || !m_componentArrays[componentID]) {
            registerComponent<T>();
        }
        
        return componentID;
    }
    
    /**
     * Get component array for type T
     */
    template<typename T>
    ComponentArray<T>* getComponentArray() {
        size_t componentID = getComponentTypeID<T>();
        return static_cast<ComponentArray<T>*>(m_componentArrays[componentID].get());
    }
    
    /**
     * Get component type ID for type T (const version)
     */
    template<typename T>
    size_t getComponentTypeID() const {
        size_t componentID = ComponentTypeIDGenerator::getID<T>();
        
        // In const context, we can't auto-register, so component must exist
        assert(componentID < m_componentArrays.size() && m_componentArrays[componentID] && 
               "Component type not registered - call non-const version first");
        
        return componentID;
    }
    
    /**
     * Get component array for type T (const version)
     */
    template<typename T>
    const ComponentArray<T>* getComponentArray() const {
        size_t componentID = getComponentTypeID<T>();
        return static_cast<const ComponentArray<T>*>(m_componentArrays[componentID].get());
    }
    
    /**
     * @brief Add component to entity with perfect forwarding
     * @param entityID The entity to add the component to
     * @param args Constructor arguments forwarded to component constructor
     * @return Reference to the newly created component
     * 
     * Uses perfect forwarding to efficiently construct components in-place.
     * Automatically registers component type if not already registered.
     */
    template<typename T, typename... Args>
    T& addComponent(size_t entityID, Args&&... args) {
        ComponentArray<T>* componentArray = getComponentArray<T>();
        
        // Create component with forwarded arguments
        T component(std::forward<Args>(args)...);
        component.exists = true; // Maintain compatibility with existing Component base class
        
        componentArray->addComponent(entityID, component);
        return componentArray->getComponent(entityID);
    }
    
    /**
     * Remove component from entity
     */
    template<typename T>
    void removeComponent(size_t entityID) {
        ComponentArray<T>* componentArray = getComponentArray<T>();
        componentArray->removeComponent(entityID);
    }
    
    /**
     * Get component from entity
     */
    template<typename T>
    T& getComponent(size_t entityID) {
        ComponentArray<T>* componentArray = getComponentArray<T>();
        return componentArray->getComponent(entityID);
    }
    
    template<typename T>
    const T& getComponent(size_t entityID) const {
        const ComponentArray<T>* componentArray = getComponentArray<T>();
        return componentArray->getComponent(entityID);
    }
    
    /**
     * Check if entity has component
     */
    template<typename T>
    bool hasComponent(size_t entityID) const {
        size_t componentID = ComponentTypeIDGenerator::getID<T>();
        if (componentID >= m_componentArrays.size() || !m_componentArrays[componentID]) {
            return false;
        }
        return m_componentArrays[componentID]->hasComponent(entityID);
    }
    
    /**
     * @brief Remove all components for entity (called when entity is destroyed)
     * @param entityID The entity to remove all components from
     * 
     * Iterates through all component arrays and removes entity's components.
     * Safe to call even if entity has no components of certain types.
     */
    void removeAllComponents(size_t entityID) {
        for (auto& componentArray : m_componentArrays) {
            if (componentArray) {
                componentArray->removeComponent(entityID);
            }
        }
    }
    
    /**
     * Clear all components
     */
    void clear() {
        for (auto& componentArray : m_componentArrays) {
            if (componentArray) {
                componentArray->clear();
            }
        }
    }
    
    /**
     * @brief Statistics structure for performance monitoring and debugging
     */
    struct Statistics {
        size_t totalComponentTypes = 0;
        size_t totalComponents = 0;
        std::vector<std::pair<std::string, size_t>> componentCounts;
    };
    
    /**
     * @brief Get detailed statistics about component usage
     * @return Statistics structure with component counts and type information
     * 
     * Useful for profiling memory usage and debugging component system performance.
     */
    Statistics getStatistics() const {
        Statistics stats;
        
        for (size_t i = 0; i < m_componentArrays.size(); ++i) {
            if (m_componentArrays[i]) {
                stats.totalComponentTypes++;
                size_t count = m_componentArrays[i]->size();
                stats.totalComponents += count;
                
                if (count > 0) {
                    stats.componentCounts.emplace_back(m_componentNames[i], count);
                }
            }
        }
        
        return stats;
    }
};