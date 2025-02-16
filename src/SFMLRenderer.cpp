#include "../include/SFMLRenderer.h"
#include <SFML/Graphics.hpp>
#include <SFML/Graphics/Drawable.hpp>
#include <SFML/Graphics/RenderWindow.hpp>

SFMLRenderer::SFMLRenderer(sf::RenderWindow &window) : m_window(window){};

void SFMLRenderer::render() {
  m_window.clear();
  // render default scene/background, etc.
  m_window.display();
}

void SFMLRenderer::render(const EntityVec &entities) {
  m_window.clear(sf::Color::Black);
  for (auto &e : entities) {
    if (e->has<CShape>()) {
      e->get<CShape>().circle.setPosition(e->get<CTransform>().pos);
      e->get<CShape>().circle.setRotation(e->get<CTransform>().angle);
      m_window.draw(e->get<CShape>().circle);
      // other drawables, sprites, mesh, etc.
    }
  }
  m_window.display();
}
