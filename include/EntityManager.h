#pragma once
#include "Entity.hpp"
#include <map>
#include <memory>
#include <string>
#include <vector>

typedef std::vector<std::shared_ptr<Entity>> EntityVec;
typedef std::map<std::string, EntityVec> EntityMap;
// TODO: abandon std::string in favor of enums

// template <typename T> bool has() const { return has_impl<T>(std::make_) }

class EntityManager {
  EntityVec m_entities;
  EntityMap m_entityMap;
  size_t m_totalEntities = 0;

private:
public:
  EntityVec m_toAdd;
  EntityManager() = default;
  void update(); // handles adding and removing
  std::shared_ptr<Entity> addEntity(const std::string &tag);
  EntityVec &getEntities();                       // all entities
  EntityVec &getEntities(const std::string &tag); // from map
  void clear();
};
