#include "../include/OpenGLRenderer.hpp"
#include "../include/FileLoader.h"
#include <SFML/Graphics/RenderWindow.hpp>
#include <glm/ext/matrix_float4x4.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>
#include <memory>
#include <ostream>

// sample vertex and fragment shaders:
const char *vertexShaderSource =
    FileLoader::loadFileAsCharPtr("./ColorShader.vert");

const char *fragmentShaderSource =
    FileLoader::loadFileAsCharPtr("./DepthFragment.frag");

std::shared_ptr<Camera> OpenGLRenderer::camera() { return m_camera; };

OpenGLRenderer::OpenGLRenderer(std::shared_ptr<Camera> camera,
                               sf::RenderWindow &window)
    : m_camera(camera), m_window(window) {
  m_window.setActive(true); // active opengl context
  if (!gladLoadGL()) {
    std::cerr << "Failed to initialize OpenGL!" << std::endl;
    return;
  }
  init();
};

OpenGLRenderer::~OpenGLRenderer() {
  glDeleteVertexArrays(1, &VAO);
  glDeleteBuffers(1, &VBO);
  glDeleteProgram(shaderProgram);
};

void OpenGLRenderer::init() {
  std::cout << "init called" << std::endl;
  // compile shaders
  unsigned int vertexShader =
      compileShader(vertexShaderSource, GL_VERTEX_SHADER);
  unsigned int fragmentShader =
      compileShader(fragmentShaderSource, GL_FRAGMENT_SHADER);

  // create shader program
  shaderProgram = glCreateProgram();
  glAttachShader(shaderProgram, vertexShader);
  glAttachShader(shaderProgram, fragmentShader);
  glLinkProgram(shaderProgram);

  // check for linking errors
  int success;
  glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
  if (!success) {
    char infoLog[512];
    glGetProgramInfoLog(shaderProgram, 512, nullptr, infoLog);
    std::cerr << "Shader program linking failed: \n" << infoLog << std::endl;
  }

  glDeleteShader(vertexShader);
  glDeleteShader(fragmentShader);
  // setupTriangle();
  glEnable(GL_DEPTH_TEST);
};

void OpenGLRenderer::render() {};

// render a single triangle
void OpenGLRenderer::render(const EntityVec &entities) {
  glClearColor(0.2f, 0.3f, 0.3f, 1.0f); // dark grey-green
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  glUseProgram(shaderProgram);
  for (const auto &e : entities) {
    if (e->has<CTriangle>() && e->has<CTransform3D>()) {
      auto &triangle = e->get<CTriangle>();
      auto &transform = e->get<CTransform3D>();

      // generating buffers on the fly, improve in the future
      glGenVertexArrays(1, &VAO);
      glGenBuffers(1, &VBO);
      glBindVertexArray(VAO);
      glBindBuffer(GL_ARRAY_BUFFER, VBO);
      glBufferData(GL_ARRAY_BUFFER, 18 * sizeof(float),
                   triangle.vertices.data(), GL_STATIC_DRAW);
      glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float),
                            (void *)0);
      glEnableVertexAttribArray(0);
      glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float),
                            (void *)(3 * sizeof(float)));
      glEnableVertexAttribArray(1);

      // matrix transforms for model, view and projection space
      glm::mat4 model = glm::mat4(1.0f);
      model = glm::translate(model, transform.position);
      model = glm::rotate(model, glm::radians(transform.rotation.x),
                          glm::vec3(1.0f, 0.0f, 0.0f));
      model = glm::rotate(model, glm::radians(transform.rotation.y),
                          glm::vec3(0.0f, 1.0f, 0.0f));
      model = glm::rotate(model, glm::radians(transform.rotation.z),
                          glm::vec3(0.0f, 0.0f, 1.0f));
      model = glm::scale(model, transform.scale);
      glm::mat4 view = m_camera->getViewMatrix();
      glm::mat4 projection = m_camera->getProjectionMatrix(1280.0f / 720.0f);

      // update uniform locations
      GLint modelLoc = glGetUniformLocation(shaderProgram, "model");
      GLint viewLoc = glGetUniformLocation(shaderProgram, "view");
      GLint projLoc = glGetUniformLocation(shaderProgram, "projection");
      glad_glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
      glad_glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
      glad_glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));

      // draw call
      glDrawArrays(GL_TRIANGLES, 0, 3);

      glBindBuffer(GL_ARRAY_BUFFER, 0);
      glBindVertexArray(0);
      glDeleteVertexArrays(1, &VAO);
    }
  }
};

// shader compilation helper
unsigned int OpenGLRenderer::compileShader(const char *source, GLenum type) {
  unsigned int shader = glCreateShader(type);
  glShaderSource(shader, 1, &source, nullptr);
  glCompileShader(shader);

  int success;
  glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
  if (!success) {
    char infoLog[512];
    glGetShaderInfoLog(shader, 512, nullptr, infoLog);
    std::cerr << "Shader compilation failed: \n" << infoLog << std::endl;
  }
  return shader;
};

void OpenGLRenderer::onUnload() {
  glBindVertexArray(0);
  glUseProgram(0);
  // glBindFramebuffer(GL_FRAMEBUFFER, 0); // ?
  // glDeleteTextures(1, &textureID);
  glDeleteBuffers(1, &VBO);
  // glDeleteFramebuffers(1, &FBO);
};
