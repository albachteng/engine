#include "Entity.cpp"
#include <algorithm>
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
  EntityVec m_toAdd;
  EntityVec m_toDelete;
  EntityMap m_entityMap;
  size_t m_totalEntities = 0;

private:
public:
  EntityManager() = default;
  void update(); // handles adding and removing
  std::shared_ptr<Entity> addEntity(const std::string &tag);
  EntityVec &getEntities();                       // all entities
  EntityVec &getEntities(const std::string &tag); // from map
};

std::shared_ptr<Entity> EntityManager::addEntity(const std::string &tag) {
  // create a new entity object
  auto e = std::shared_ptr<Entity>(new Entity(m_totalEntities++, tag));
  m_toAdd.push_back(e); // delay for iterator invalidation
  return e;
}

void EntityManager::update() {
  for (auto e : m_toAdd) {
    m_entities.push_back(e);
    m_entityMap[e->tag()].push_back(e);
  }
  for (auto e : m_toDelete) {
    auto it = std::remove_if(
        m_entities.begin(), m_entities.end(),
        [](const std::shared_ptr<Entity> &e) { return !e->isAlive(); });
    m_entities.erase(it, m_entities.end());
    auto another = std::remove_if(
        m_entityMap[e->tag()].begin(), m_entityMap[e->tag()].end(),
        [](const std::shared_ptr<Entity> &e) { return !e->isAlive(); });
    m_entities.erase(another, m_entityMap[e->tag()].end());
  }
  m_toAdd.clear();
}

EntityVec &EntityManager::getEntities() { return m_entities; }
EntityVec &EntityManager::getEntities(const std::string &tag) {
  return m_entityMap[tag];
}

/* usage example:
 * void spawnEnemy() {
 * auto e = m_entities.addEntity("enemy")
 * e->add<CTransform>(args);
 * e->add<CShape>(args);
 * }
 * */
