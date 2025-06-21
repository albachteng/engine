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
  EntityManager() = default;
  void update(); // handles adding and removing
  std::shared_ptr<Entity> addEntity(const EntityTag &tag);
  EntityVec &getEntities();                       // all entities
  EntityVec &getEntities(const EntityTag &tag); // from map
  const EntityVec &getEntities() const;                       // all entities (const)
  const EntityVec &getEntities(const EntityTag &tag) const; // from map (const)
  bool hasTag(const EntityTag &tag) const;                  // check if tag exists
  void clear();
};
