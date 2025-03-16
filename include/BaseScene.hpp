#pragma once
#include "InputEvent.hpp"
#include <SFML/Graphics/RenderWindow.hpp>

class BaseScene {
public:
  virtual ~BaseScene() = default;
  virtual void onLoad() = 0;
  virtual void onUnload() = 0;
  virtual void update(float deltaTime) = 0;
  virtual void processInput(const InputEvent &type, float deltaTime = 0.0f) = 0;

  virtual void sMovement(float deltaTime) = 0;
  virtual void sInput(sf::Event &event, float deltaTime) = 0;
  virtual void sRender() = 0;
  virtual bool isPaused() const { return false; };
};
