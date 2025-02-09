#pragma once
#include <SFML/Graphics.hpp>
#include <SFML/Graphics/CircleShape.hpp>
#include <SFML/System/Vector2.hpp>

typedef sf::Vector2f Vec2f;

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
