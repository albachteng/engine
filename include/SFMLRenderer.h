#pragma once
#include "../include/EntityManager.h"
#include "../include/Renderer.h"
#include <SFML/Graphics/RenderWindow.hpp>

class SFMLRenderer : public Renderer {
private:
  sf::RenderWindow &m_window;
  
  // Specialized rendering methods
  void renderPolygon(const CComplexShape& complexShape, const CTransform& transform);
  sf::ConvexShape createPolygonShape(const std::vector<Vec2f>& vertices);

public:
  explicit SFMLRenderer(sf::RenderWindow &window);
  ~SFMLRenderer();
  void init() override;
  void render() override;
  void render(const EntityVec &entities) override;
};
