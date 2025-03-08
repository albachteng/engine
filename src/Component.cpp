#include "../include/Component.h"

CTransform::CTransform() = default;
CTransform::CTransform(const Vec2f &p, const Vec2f &v, float a)
    : pos(p), vel(v), angle(a){};

CTransform3D::CTransform3D() = default;
CTransform3D::CTransform3D(const glm::vec3 &position, const glm::vec3 &rotation,
                           const glm::vec3 &scale)
    : position(position), rotation(rotation), scale(scale){};

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

CBoundingBox::CBoundingBox(const Vec2f &size)
    : size(size), halfsize(Vec2f{size.x / 2.0f, size.y / 2.0f}){};

CGravity::CGravity() = default;

CTriangle::CTriangle() = default;

CAABB::CAABB() = default;
CAABB::CAABB(const glm::vec3 &center, const glm::vec3 &halfsize) {
  min = center - halfsize;
  max = center + halfsize;
}

CMovement3D::CMovement3D() = default;

CMovement3D::CMovement3D(const glm::vec3 &velocity,
                         const glm::vec3 &acceleration)
    : vel(velocity), acc(acceleration){};
