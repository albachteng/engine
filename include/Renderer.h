#pragma once
#include "EntityManager.h"

class Renderer {
public:
  virtual ~Renderer() = default;
  virtual void init() = 0;
  virtual void render(const EntityVec &entities) = 0;
  virtual void render() = 0;
};
