#include "../include/SFMLRenderer.h"
#include <SFML/Graphics.hpp>
#include <SFML/Graphics/Drawable.hpp>
#include <SFML/Graphics/RenderWindow.hpp>

SFMLRenderer::SFMLRenderer(){};

void SFMLRenderer::render(sf::RenderWindow &window, EntityVec entities) {
  window.clear();

  for (auto &e : entities) {
    if (e->has<CShape>()) {
      window.draw(e->get<CShape>().circle);
    }
  }
  window.display();
};
