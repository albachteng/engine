#include <map>
#include <memory>
#include <string>
#include <vector>

class Entity {};

typedef std::vector<std::shared_ptr<Entity>> EntityVec;
typedef std::map<std::string, EntityVec> EntityMap;
// TODO: abandon std::string in favor of enums

class EntityManager {
  EntityVec m_entities;
  EntityMap m_entityMap;
  size_t m_totalEntities = 0;

private:
public:
  EntityManager();
  std::shared_ptr<Entity> addEntity(const std::string &tag);
  EntityVec &getEntities();
  EntityVec &getEntities(const std::string &tag);
};

std::shared_ptr<Entity> EntityManager::addEntity(const std::string &tag) {
  // create a new entity object
  auto e = std::make_shared<Entity>(tag, m_totalEntities++);
  m_entities.push_back(e);
  m_entityMap[tag].push_back(e);
  return e;
}

/* usage example:
 * void spawnEnemy() {
 * auto e = m_entities.addEntity("enemy")
 * e->add<CTransform>(args);
 * e->add<CShape>(args);
 * }
 * */
