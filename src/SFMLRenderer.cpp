#include "../include/SFMLRenderer.h"
#include <SFML/Graphics.hpp>
#include <SFML/Graphics/Drawable.hpp>
#include <SFML/Graphics/RenderWindow.hpp>

SFMLRenderer::SFMLRenderer(sf::RenderWindow &window) : m_window(window){};
SFMLRenderer::~SFMLRenderer(){};

void SFMLRenderer::render() {
  // render default scene/background, etc.
}

void SFMLRenderer::init() {};

void SFMLRenderer::render(const EntityVec &entities) {
  for (auto &e : entities) {
    if (e->has<CShape>()) {
      auto transform = e->get<CTransform>();
      auto &shape = e->get<CShape>();
      e->get<CShape>().circle.setPosition(transform.pos);
      e->get<CShape>().circle.setRotation(transform.angle);
      m_window.draw(shape.circle);
    }
    else if (e->has<CComplexShape>()) {
      auto transform = e->get<CTransform>();
      auto &complexShape = e->get<CComplexShape>();
      
      if (complexShape.type == CComplexShape::POLYGON || 
          complexShape.type == CComplexShape::VORONOI_REGION) {
        renderPolygon(complexShape, transform);
      }
    }
  }
}

void SFMLRenderer::renderPolygon(const CComplexShape& complexShape, const CTransform& transform) {
  if (complexShape.vertices.empty()) return;
  
  sf::ConvexShape polygon = createPolygonShape(complexShape.vertices);
  
  // Apply colors and styling
  polygon.setFillColor(complexShape.fillColor);
  polygon.setOutlineColor(complexShape.outlineColor);
  polygon.setOutlineThickness(complexShape.outlineThickness);
  
  // Apply transform (note: polygon vertices are already in world coordinates)
  polygon.setPosition(0, 0); // Don't offset, vertices are already positioned
  polygon.setRotation(transform.angle);
  
  m_window.draw(polygon);
  
  // Debug: draw vertices if enabled
  if (complexShape.showVertices) {
    for (const auto& vertex : complexShape.vertices) {
      sf::CircleShape dot(2.0f);
      dot.setFillColor(sf::Color::Red);
      dot.setPosition(vertex.x - 2.0f, vertex.y - 2.0f);
      m_window.draw(dot);
    }
  }
}

sf::ConvexShape SFMLRenderer::createPolygonShape(const std::vector<Vec2f>& vertices) {
  sf::ConvexShape polygon;
  polygon.setPointCount(vertices.size());
  
  for (size_t i = 0; i < vertices.size(); ++i) {
    polygon.setPoint(i, sf::Vector2f(vertices[i].x, vertices[i].y));
  }
  
  return polygon;
}
