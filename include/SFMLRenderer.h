#pragma once
#include "../include/EntityManager.h"
#include "../include/Renderer.h"
#include <SFML/Graphics/RenderWindow.hpp>

class SFMLRenderer : public Renderer {

public:
  explicit SFMLRenderer();
  void render(sf::RenderWindow &window, EntityVec entities);
};
