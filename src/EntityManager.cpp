#include "../include/EntityManager.h"
#include <algorithm>
#include <iostream>
#include <map>
#include <memory>
#include <string>
#include <vector>

typedef std::vector<std::shared_ptr<Entity>> EntityVec;
typedef std::map<std::string, EntityVec> EntityMap;
// TODO: abandon std::string in favor of enums

// template <typename T> bool has() const { return has_impl<T>(std::make_) }

std::shared_ptr<Entity> EntityManager::addEntity(const std::string &tag) {
  // create a new entity object
  auto e = std::shared_ptr<Entity>(new Entity(m_totalEntities++, tag));
  std::cout << "adding entity with tag: " << tag << std::endl;
  m_toAdd.push_back(e); // delay for iterator invalidation
  return e;
};

void EntityManager::update() {

  for (auto e : m_toAdd) {
    std::cout << "pushing entity in update: " << e->tag() << std::endl;
    m_entities.push_back(e);
    m_entityMap[e->tag()].push_back(e);
  }
  for (auto e : m_entities) {
    auto it = std::remove_if(
        m_entities.begin(), m_entities.end(),
        [](const std::shared_ptr<Entity> &e) { return !e->isActive(); });
    m_entities.erase(it, m_entities.end());
    auto another = std::remove_if(
        m_entityMap[e->tag()].begin(), m_entityMap[e->tag()].end(),
        [](const std::shared_ptr<Entity> &e) { return !e->isActive(); });
    m_entities.erase(another, m_entityMap[e->tag()].end());
  }
  m_toAdd.clear();
};

EntityVec &EntityManager::getEntities() { return m_entities; };
EntityVec &EntityManager::getEntities(const std::string &tag) {
  return m_entityMap[tag];
};

/* usage example:
 * void spawnEnemy() {
 * auto e = m_entities.addEntity("enemy")
 * e->add<CTransform>(args);
 * e->add<CShape>(args);
 * }
 * */
