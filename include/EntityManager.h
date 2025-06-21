#pragma once
#include "Entity.hpp"
#include <map>
#include <memory>
#include <string>
#include <vector>

typedef std::vector<std::shared_ptr<Entity>> EntityVec;
typedef std::map<EntityTag, EntityVec> EntityMap;

// template <typename T> bool has() const { return has_impl<T>(std::make_) }

class EntityManager {
private:
  EntityVec m_entities;
  EntityMap m_entityMap;
  EntityVec m_toAdd;
  size_t m_totalEntities = 0;

public:
  EntityManager() {
    // Initialize component system
    Entity::initializeComponentManager();
  }
  
  ~EntityManager() {
    // Clean up component system
    Entity::shutdownComponentManager();
  }
  /**
   * @brief Process deferred entity additions and removals for frame consistency
   * 
   * Must be called once per frame to handle entities marked for addition/removal.
   * Ensures entities are not modified during iteration over entity collections.
   */
  void update();
  
  /**
   * @brief Create a new entity with the specified tag (deferred addition)
   * @param tag The tag to assign to the new entity
   * @return Shared pointer to the newly created entity
   * 
   * Entity is not immediately added to active collections - call update() to process.
   */
  std::shared_ptr<Entity> addEntity(const EntityTag &tag);
  
  EntityVec &getEntities();                       // all entities
  EntityVec &getEntities(const EntityTag &tag); // from map
  const EntityVec &getEntities() const;                       // all entities (const)
  const EntityVec &getEntities(const EntityTag &tag) const; // from map (const)
  
  /**
   * @brief Find entity by unique ID
   * @param id The unique entity ID to search for
   * @return Shared pointer to entity if found, nullptr otherwise
   */
  std::shared_ptr<Entity> getEntityById(size_t id);
  std::shared_ptr<Entity> getEntityById(size_t id) const;
  
  /**
   * @brief Check if any entities exist with the specified tag
   * @param tag The entity tag to check
   * @return True if at least one entity has this tag
   */
  bool hasTag(const EntityTag &tag) const;
  
  /**
   * @brief Remove all entities and reset the manager to initial state
   * 
   * Clears all active entities, pending additions, and component data.
   */
  void clear();
};
