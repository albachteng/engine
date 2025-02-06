#include <string>
class Entity {

private:
  Entity(const size_t &id, const std::string &tag) : m_tag(tag), m_id(id){};
  // ComponentTuple m_components;
  bool m_alive = true;
  std::string m_tag = "default";
  size_t m_id = 0;

  friend class EntityManager;

public:
  // template <typename T> void add<T>(args);
  // template <typename T> T &get<T>();
  size_t id() const { return m_id; };
  bool isAlive() const { return m_alive; };
  void destroy() { m_alive = false; };
  const std::string &tag() const { return m_tag; };
  //   template <typename T> bool has() const { return get<T>().exists; }
  // template <typename T, typename... TArgs> T &add(TArgs &&...mArgs) {
  // auto &component = get<T>();
  // component = T(std::forward<TArgs>(mArgs)...);
  // component.exists = true;
  // return component;
  // };
};
