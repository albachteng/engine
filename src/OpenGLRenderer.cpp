#include "../include/OpenGLRenderer.hpp"
#include <SFML/Graphics/RenderWindow.hpp>
#include <glm/ext/matrix_float4x4.hpp>
#include <glm/ext/vector_float3.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>
#include <memory>
#include <ostream>

// sample vertex and fragment shaders:
const char *vertexShaderSource = R"(
#version 330 core
layout (location = 0) in vec3 aPos;
uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main() {
  gl_Position = projection * view * model * vec4(aPos, 1.0);
})";

const char *fragmentShaderSource = R"(
#version 330 core
out vec4 FragColor;
void main() {
  FragColor = vec4(0.5, 0.2, 0.8, 1.0);
})";

std::shared_ptr<Camera> OpenGLRenderer::camera() { return m_camera; };

OpenGLRenderer::OpenGLRenderer(std::shared_ptr<Camera> camera,
                               sf::RenderWindow &window)
    : m_window(window), m_camera(camera) {
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
  std::cout << "setupTriangle is commented out" << std::endl;
  glEnable(GL_DEPTH_TEST);
};

// TODO: move triangle setup into entity manager
void OpenGLRenderer::setupTriangle() {
  // float vertices[] = {
  //     -0.5f, -0.5f, 0.0f, // bottom left
  //     0.5f,  -0.5f, 0.0f, // bottom right
  //     0.0f,  0.5f,  0.0f, // top center
  // };
  //
  // glm::mat4 model =
  //     glm::translate(glm::mat4(1.0f), glm::vec3(1.0f, 1.0f, 0.0f));
  // glGenVertexArrays(1, &VAO);
  // glGenBuffers(1, &VBO);
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

      std::cout << "do we have a triangle?" << std::endl;
      // generating buffers on the fly, improve in the future
      glGenVertexArrays(1, &VAO);
      glGenBuffers(1, &VBO);
      glBindVertexArray(VAO);
      glBindBuffer(GL_ARRAY_BUFFER, VBO);
      glBufferData(GL_ARRAY_BUFFER, triangle.vertices.size() * sizeof(float),
                   triangle.vertices.data(), GL_STATIC_DRAW);
      glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float),
                            (void *)0);
      glEnableVertexAttribArray(0);

      // TODO: model matrix to be computed based on entity's CTransform
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
      // glBindVertexArray(VAO);
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
