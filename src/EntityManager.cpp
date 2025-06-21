#include "../include/EntityManager.h"
#include <algorithm>
#include <iostream>
#include <map>
#include <memory>
#include <string>
#include <vector>

typedef std::vector<std::shared_ptr<Entity>> EntityVec;
typedef std::map<EntityTag, EntityVec> EntityMap;

// template <typename T> bool has() const { return has_impl<T>(std::make_) }

std::shared_ptr<Entity> EntityManager::addEntity(const EntityTag &tag) {
  // Helper struct to access private constructor with make_shared
  struct EntityBuilder : public Entity {
    EntityBuilder(size_t id, const EntityTag &tag) : Entity(id, tag) {}
  };
  
  auto e = std::make_shared<EntityBuilder>(m_totalEntities++, tag);
  m_toAdd.push_back(e); // delay for iterator invalidation
  return e;
};

void EntityManager::update() {

  for (auto e : m_toAdd) {
    m_entities.push_back(e);
    m_entityMap[e->tag()].push_back(e);
  }
  // Remove inactive entities from main vector
  auto it = std::remove_if(
      m_entities.begin(), m_entities.end(),
      [](const std::shared_ptr<Entity> &e) { return !e->isActive(); });
  m_entities.erase(it, m_entities.end());

  // Remove inactive entities from each tag group
  for (auto &pair : m_entityMap) {
    auto &taggedEntities = pair.second;
    auto tagIt = std::remove_if(
        taggedEntities.begin(), taggedEntities.end(),
        [](const std::shared_ptr<Entity> &e) { return !e->isActive(); });
    taggedEntities.erase(tagIt, taggedEntities.end());
  }
  m_toAdd.clear();
};

EntityVec &EntityManager::getEntities() { return m_entities; }

EntityVec &EntityManager::getEntities(const EntityTag &tag) {
  return m_entityMap[tag];
}

const EntityVec &EntityManager::getEntities() const { return m_entities; }

const EntityVec &EntityManager::getEntities(const EntityTag &tag) const {
  auto it = m_entityMap.find(tag);
  if (it != m_entityMap.end()) {
    return it->second;
  }
  static const EntityVec empty;
  return empty;
}

bool EntityManager::hasTag(const EntityTag &tag) const {
  return m_entityMap.find(tag) != m_entityMap.end();
}

void EntityManager::clear() {
  m_entities.clear();
  m_entityMap.clear();
  m_toAdd.clear();
  m_totalEntities = 0;
};

/* usage example:
 * void spawnEnemy() {
 * auto e = m_entities.addEntity(EntityTag::ENEMY)
 * e->add<CTransform>(args);
 * e->add<CShape>(args);
 * }
 * */
