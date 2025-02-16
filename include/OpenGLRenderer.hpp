#pragma once

#include "./Renderer.h"
#include "EntityManager.h"
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Window.hpp>
#include <glad/glad.h>

class OpenGLRenderer : public Renderer {
public:
  OpenGLRenderer(sf::RenderWindow &window);
  ~OpenGLRenderer();

  void init();
  void render() override;
  void render(const EntityVec &entities) override;

private:
  sf::RenderWindow &m_window;
  unsigned int VAO, VBO, shaderProgram;

  void setupTriangle();
  unsigned int compileShader(const char *source, GLenum type);
};
