#pragma once
#include "Constants.hpp"
#include <SFML/Graphics.hpp>
#include <SFML/Graphics/CircleShape.hpp>
#include <SFML/System/Vector2.hpp>
#include <glm/ext/vector_float3.hpp>

typedef sf::Vector2f Vec2f;
typedef sf::Vector2i Vec2i;

class Component {
public:
  bool exists = false;
};

class CTransform : public Component {
public:
  Vec2f pos = {0.0, 0.0};
  Vec2f vel = {0.0, 0.0};
  float angle = 0;
  CTransform();
  CTransform(const Vec2f &p, const Vec2f &v, float a);
};

class CTransform3D : public Component {
public:
  glm::vec3 position = {0.0f, 0.0f, 0.0f};
  glm::vec3 rotation = {0.0f, 0.0f, 0.0f};
  glm::vec3 scale = {1.0f, 1.0f, 1.0f};
  CTransform3D();
  CTransform3D(const glm::vec3 &position, const glm::vec3 &rotation,
               const glm::vec3 &scale);
};

class CShape : public Component {
public:
  sf::CircleShape circle;
  CShape();
  CShape(float radius, size_t points, const sf::Color &fill,
         const sf::Color &outline, float thickness);
};

class CCollision : public Component {
private:
public:
  float radius = 0;
  CCollision();
  CCollision(float r);
};

class CScore : public Component {
public:
  int score = 0;
  CScore();
  CScore(int s);
};

class CLifespan : public Component {
public:
  int lifespan = 0;
  int remaining = 0;
  CLifespan();
  CLifespan(int totalLifespan);
};

class CInput : public Component {
public:
  bool up = false;
  bool down = false;
  bool left = false;
  bool right = false;
  bool shoot = false;
  CInput();
};

class CBoundingBox : public Component {
public:
  Vec2f size;
  Vec2f halfsize;
  CBoundingBox(const Vec2f &s);
};

class CGravity : public Component {
public:
  Vec2f gravity = {EngineConstants::Physics::GRAVITY_X, EngineConstants::Physics::GRAVITY_Y};
  CGravity();
};

class CTriangle : public Component {
  // A simple triangle in 3D
  // 18 floats: 3 vertices, each with x, y, z, R, G, B float values
public:
  std::vector<float> vertices = {
      // vertices		  // colors
      -0.5f, -0.5f, 0.0f, 1.0f, 0.0f, 0.0f, // bottom left
      0.5f,  -0.5f, 0.0f, 0.0f, 0.0f, 1.0f, // bottom right
      0.0f,  0.5f,  0.0f, 0.0f, 1.0f, 0.0f  // top center
  };
  CTriangle();
  // Optionally, add indices, colors, etc.
};

class CAABB : public Component {
public:
  glm::vec3 min;
  glm::vec3 max;

  CAABB();
  CAABB(const glm::vec3 &center, const glm::vec3 &halfsize);
};

class CMovement3D : public Component {
public:
  glm::vec3 vel;
  glm::vec3 acc;

  CMovement3D();
  CMovement3D(const glm::vec3 &velocity, const glm::vec3 &acceleration);
};

class CSelection : public Component {
public:
  Vec2i grid_position;

  CSelection();
  CSelection(Vec2i pos);
};
