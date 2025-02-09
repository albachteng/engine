#pragma once
#include "Component.h"
#include <tuple>

typedef std::tuple<CTransform, CShape, CCollision, CInput, CScore, CLifespan>
    ComponentTuple;

class Entity {

private:
  Entity(const size_t &id, const std::string &tag) : m_tag(tag), m_id(id){};
  ComponentTuple m_components;
  bool m_active = true;
  std::string m_tag = "default";
  size_t m_id = 0;

  friend class EntityManager;

public:
  size_t id() const;
  bool isActive() const;
  void destroy();
  const std::string &tag() const;

  template <typename T> T &get() { return std::get<T>(m_components); };
  template <typename T> bool has() const { return get<T>().exists; }
  template <typename T, typename... TArgs> T &add(TArgs &&...mArgs) {
    auto &component = get<T>();
    component = T(std::forward<TArgs>(mArgs)...);
    component.exists = true;
    return component;
  };
  template <typename T> void remove() { get<T>() = T(); };
};
