#pragma once
#include <SFML/Window/Event.hpp>
#include <functional>
#include <iostream>
#include <unordered_map>

template <typename Action> class ActionController {
private:
  std::unordered_map<Action, std::function<void(float)>> m_listeners;
  std::unordered_map<Action, std::function<void(float, float)>> m_axisListeners;

public:
  ActionController<Action>() = default;

  void unregisterAll() {
    m_listeners.clear();
    m_axisListeners.clear();
  };

  void registerListener(const Action &action,
                        std::function<void(float)> listener) {
    m_listeners[action] = listener;
  };

  void registerAxisListener(const Action &action,
                            std::function<void(float, float)> listener) {
    m_axisListeners[action] = listener;
  };

  void handleEvent(const Action &action, float deltaTime = 0.0f,
                   float xOffset = 0.0f, float yOffset = 0.0f) {
    // std::cout << "handleEvent" << std::endl;
    if (m_listeners.find(action) != m_listeners.end())
      m_listeners[action](deltaTime);
    if (m_axisListeners.find(action) != m_axisListeners.end()) {
      // auto [x, y] = std::get<std::pair<float, float>>(action.data);
      m_axisListeners[action](xOffset, yOffset);
    }
  };
};
