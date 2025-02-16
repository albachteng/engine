#pragma once
#include <any>
#include <iostream>
#include <map>
#include <optional>
#include <ostream>

template <typename ActionName, typename ActionType> class BaseScene {
protected:
  // Use std::any to allow different enums in derived classes
  std::map<int, ActionName> m_actionMap;
  // Entity m_player;
  virtual void handleAction(const ActionName &action,
                            const ActionType &type) = 0;

public:
  virtual ~BaseScene() = default;

  virtual void init() = 0; // Derived classes register actions here

  std::map<int, ActionName> getActionMap() { return m_actionMap; };

  void registerAction(int key, ActionName action_name) {
    std::cout << "registering: " << key << std::endl;
    m_actionMap[key] = action_name;
  }

  std::optional<ActionName> getAction(int key) {
    if (m_actionMap.find(key) != m_actionMap.end()) {
      return std::any_cast<ActionName>(m_actionMap[key]);
    }
    return std::nullopt; // Return empty if key not found
  }

  void doAction(const ActionName &action, const ActionType &type) {
    handleAction(action, type);
  }
};
