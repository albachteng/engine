#pragma once
#include <SFML/Window/Keyboard.hpp>
#include <SFML/Window/Mouse.hpp>
#include <cstddef>
#include <functional>
#include <variant>

enum class InputType {
  Keyboard,
  MouseButton,
  MouseMove,
  ControllerButton,
  ControllerAxis
};

struct InputEvent {
  InputType type;
  std::variant<sf::Keyboard::Key, sf::Mouse::Button, std::pair<float, float>,
               int>
      data;

  bool operator==(const InputEvent &other) const {
    return type == other.type && data == other.data;
  };
};

namespace std {
template <> struct hash<InputEvent> {
  std::size_t operator()(const InputEvent &event) const {
    return hash<int>()(static_cast<int>(event.type));
  }
};
} // namespace std
