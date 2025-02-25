#pragma once
#include "../include/InputEvent.hpp"
#include <SFML/Window/Event.hpp>
#include <functional>
#include <string>
#include <unordered_map>

class IInputController {
public:
  virtual ~IInputController() = default;
  virtual void registerListener(const InputEvent &event,
                                std::function<void(float)> listener) = 0;
  virtual void
  registerAxisListener(const InputEvent &event,
                       std::function<void(float, float)> listener) = 0;
  virtual void handleEvent(const InputEvent &event, float x = 0.0f,
                           float y = 0.0f, float deltaTime = 0.0f) = 0;
};

class InputController : public IInputController {
private:
  std::unordered_map<InputEvent, std::function<void(float)>> m_listeners;
  std::unordered_map<InputEvent, std::function<void(float, float)>>
      m_axisListeners;

public:
  InputController() = default;

  void registerListener(const InputEvent &event,
                        std::function<void(float)> listener) override {
    m_listeners[event] = listener;
  };

  void
  registerAxisListener(const InputEvent &event,
                       std::function<void(float, float)> listener) override {
    m_axisListeners[event] = listener;
  };

  void handleEvent(const InputEvent &event, float x = 0.0f, float y = 0.0f,
                   float deltaTime = 0.0f) override {
    if (m_listeners.find(event) != m_listeners.end())
      m_listeners[event](deltaTime);
    if (m_axisListeners.find(event) != m_axisListeners.end())
      m_axisListeners[event](x, y);
  };
};
