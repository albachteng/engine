#pragma once
#include "../include/Action.hpp"
#include <map>
#include <string>

class Scene {
private:
  std::map<int, std::string> m_actionMap;

public:
  virtual void
  init() = 0; // abstract, registerAction(sf::Keyboard::W, "UP") etc
  void registerAction(int key, const std::string &name);
  std::map<int, std::string> getActionMap();
  void doAction(const Action &action);
};
