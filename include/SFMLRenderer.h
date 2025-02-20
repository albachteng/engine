#pragma once
#include "../include/EntityManager.h"
#include "../include/Renderer.h"
#include <SFML/Graphics/RenderWindow.hpp>

class SFMLRenderer : public Renderer {
private:
  sf::RenderWindow &m_window;

public:
  explicit SFMLRenderer(sf::RenderWindow &window);
  ~SFMLRenderer();
  void init() override;
  void render() override;
  void render(const EntityVec &entities) override;
};
