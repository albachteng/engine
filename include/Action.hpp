#pragma once
#include <any>

class Action {
  std::any m_name;
  std::any m_type;

public:
  Action() = default;
  template <typename T, typename U>
  Action(T name, U type) : m_name(name), m_type(type){};
  template <typename T> const T &name() const {
    return std::any_cast<const T &>(m_name);
  };
  template <typename T> const T &type() const {
    return std::any_cast<const T &>(m_type);
  };
};
