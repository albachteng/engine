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
      // other drawables, sprites, mesh, etc.
    }
  }
}
