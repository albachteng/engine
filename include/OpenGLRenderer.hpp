#pragma once
#include "./Renderer.h"
#include "Camera.h"
#include "EntityManager.h"
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Window.hpp>
#include <glad/glad.h>
#include <memory>

class OpenGLRenderer : public Renderer {
public:
  OpenGLRenderer(std::shared_ptr<Camera> camera, sf::RenderWindow &window);
  ~OpenGLRenderer();
  std::shared_ptr<Camera> m_camera;

  void init() override;
  void render() override;
  void render(const EntityVec &entities) override;
  std::shared_ptr<Camera> camera();

private:
  sf::RenderWindow &m_window;
  unsigned int VAO, VBO, shaderProgram;
  unsigned int compileShader(const char *source, GLenum type);
};
