#pragma once
#include "../include/EntityManager.h"
#include "../include/Renderer.h"
#include <SFML/Graphics/RenderWindow.hpp>

class SFMLRenderer : public Renderer {
private:
  sf::RenderWindow &m_window;

public:
  explicit SFMLRenderer(sf::RenderWindow &window);
  void render();
  void render(const EntityVec &entities);
};
