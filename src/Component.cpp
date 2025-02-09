#include "../include/Component.h"

CTransform::CTransform() = default;
CTransform::CTransform(const Vec2f &p, const Vec2f &v, float a)
    : pos(p), vel(v), angle(a){};

CShape::CShape() = default;
CShape::CShape(float radius, size_t points, const sf::Color &fill,
               const sf::Color &outline, float thickness)
    : circle(radius, points) {
  circle.setFillColor(fill);
  circle.setOutlineColor(outline);
  circle.setOutlineThickness(thickness);
  circle.setOrigin(radius, radius);
};

CCollision::CCollision() = default;
CCollision::CCollision(float r) : radius(r){};

CScore::CScore() = default;
CScore::CScore(int s) : score(s){};

CLifespan::CLifespan() = default;
CLifespan::CLifespan(int totalLifespan)
    : lifespan(totalLifespan), remaining(totalLifespan){};

CInput::CInput() = default;
