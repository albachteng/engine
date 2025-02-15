#pragma once
#include <any>
#include <map>
#include <optional>

template <typename ActionEnum> class BaseScene {
protected:
  // Use std::any to allow different enums in derived classes
  std::map<int, ActionEnum> m_actionMap;
  // Entity m_player;
  virtual void handleAction(const ActionEnum &action) = 0;

public:
  virtual ~BaseScene() = default;

  virtual void init() = 0; // Derived classes register actions here

  std::map<int, ActionEnum> getActionMap() { return m_actionMap; };

  void registerAction(int key, ActionEnum action_name) {
    m_actionMap[key] = action_name;
  }

  std::optional<ActionEnum> getAction(int key) {
    if (m_actionMap.find(key) != m_actionMap.end()) {
      return std::any_cast<ActionEnum>(m_actionMap[key]);
    }
    return std::nullopt; // Return empty if key not found
  }

  void doAction(const ActionEnum &action) { handleAction(action); }
};
