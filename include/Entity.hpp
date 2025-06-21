#pragma once
#include "Component.h"
#include <tuple>

enum class EntityTag {
  DEFAULT,
  TRIANGLE,
  PLAYER,
  MAP_NODE,
  ENEMY
};

typedef std::tuple<CTransform, CTransform3D, CShape, CCollision, CInput, CScore,
                   CLifespan, CGravity, CTriangle, CMovement3D, CAABB,
                   CSelection>
    ComponentTuple;

class Entity {
private:
  Entity(const size_t &id, const EntityTag &tag) : m_tag(tag), m_id(id){};
  ComponentTuple m_components;
  bool m_active = true;
  EntityTag m_tag = EntityTag::DEFAULT;
  size_t m_id = 0;

  friend class EntityManager;

public:
  size_t id() const;
  bool isActive() const;
  void destroy();
  const EntityTag &tag() const;

  template <typename T> T &get() { return std::get<T>(m_components); };
  template <typename T> bool has() { return get<T>().exists; }
  template <typename T, typename... TArgs> T &add(TArgs &&...mArgs) {
    auto &component = get<T>();
    component = T(std::forward<TArgs>(mArgs)...);
    component.exists = true;
    return component;
  };
  template <typename T> void remove() { get<T>() = T(); };
};
