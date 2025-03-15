#pragma once
#include "InputEvent.hpp"
#include "Renderer.h"
#include <any>
#include <iostream>
#include <map>
#include <memory>
#include <optional>
#include <ostream>

class BaseScene {
private:
  std::unique_ptr<Renderer> m_renderer; // TODO: RenderSystem ?

public:
  virtual ~BaseScene() = default;
  virtual void onLoad() = 0;
  virtual void onUnload() = 0;
  virtual void update(float deltaTime) = 0;
  virtual void processInput(const InputEvent &type, float deltaTime = 0.0f) = 0;
  virtual void render() = 0;
};
