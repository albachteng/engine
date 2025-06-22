#include "../include/OpenGLRenderer.hpp"
#include "../include/Constants.hpp"
#include "../include/FileLoader.h"
#include <SFML/Graphics/RenderWindow.hpp>
#include <glm/ext/matrix_float4x4.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>
#include <memory>
#include <ostream>

// Load shader sources safely using RAII
static const std::string vertexShaderSourceStr =
    FileLoader::loadFileAsString("./src/ColorShader.vert");
static const std::string fragmentShaderSourceStr =
    FileLoader::loadFileAsString("./src/DepthFragment.frag");

static const char *vertexShaderSource = vertexShaderSourceStr.c_str();
static const char *fragmentShaderSource = fragmentShaderSourceStr.c_str();

std::shared_ptr<Camera> OpenGLRenderer::camera() { return m_camera; };

OpenGLRenderer::OpenGLRenderer(std::shared_ptr<Camera> camera,
                               sf::RenderWindow &window)
    : m_camera(camera), m_window(window), VAO(0), VBO(0), shaderProgram(0),
      m_initialized(false) {
  m_window.setActive(true); // active opengl context
  if (!gladLoadGL()) {
    std::cerr << "Failed to initialize OpenGL!" << std::endl;
    return;
  }
  init();
};

OpenGLRenderer::~OpenGLRenderer() {
  if (m_initialized) {
    GL_CALL(glDeleteVertexArrays(1, &VAO));
    GL_CALL(glDeleteBuffers(1, &VBO));
    GL_CALL(glDeleteProgram(shaderProgram));
  }
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
  GL_CHECK_ERROR();
  GL_CALL(glAttachShader(shaderProgram, vertexShader));
  GL_CALL(glAttachShader(shaderProgram, fragmentShader));
  GL_CALL(glLinkProgram(shaderProgram));

  // check for linking errors
  int success;
  GL_CALL(glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success));
  if (!success) {
    char infoLog[EngineConstants::Graphics::SHADER_LOG_BUFFER_SIZE];
    GL_CALL(glGetProgramInfoLog(
        shaderProgram, EngineConstants::Graphics::SHADER_LOG_BUFFER_SIZE,
        nullptr, infoLog));
    std::cerr << "Shader program linking failed: \n" << infoLog << std::endl;
  }

  GL_CALL(glDeleteShader(vertexShader));
  GL_CALL(glDeleteShader(fragmentShader));

  setupBuffers();
  GL_CALL(glEnable(GL_DEPTH_TEST));
  m_initialized = true;
};

void OpenGLRenderer::render() {};

void OpenGLRenderer::render(const EntityVec &entities) {
  if (!m_initialized)
    return;

  GL_CALL(glClearColor(EngineConstants::Graphics::CLEAR_COLOR_R,
                       EngineConstants::Graphics::CLEAR_COLOR_G,
                       EngineConstants::Graphics::CLEAR_COLOR_B,
                       EngineConstants::Graphics::CLEAR_COLOR_A));
  GL_CALL(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));
  GL_CALL(glUseProgram(shaderProgram));

  GL_CALL(glBindVertexArray(VAO));

  for (const auto &e : entities) {
    if (e->has<CTriangle>() && e->has<CTransform3D>()) {
      auto &triangle = e->get<CTriangle>();
      auto &transform = e->get<CTransform3D>();

      // Update buffer data
      GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, VBO));
      GL_CALL(glBufferData(
          GL_ARRAY_BUFFER,
          EngineConstants::Graphics::TRIANGLE_VERTEX_DATA_SIZE * sizeof(float),
          triangle.vertices.data(), GL_DYNAMIC_DRAW));

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
      glm::mat4 projection =
          m_camera->getProjectionMatrix(EngineConstants::Display::ASPECT_RATIO);

      // update uniform locations (could be cached for better performance)
      GLint modelLoc = glGetUniformLocation(shaderProgram, "model");
      GL_CHECK_ERROR();
      GLint viewLoc = glGetUniformLocation(shaderProgram, "view");
      GL_CHECK_ERROR();
      GLint projLoc = glGetUniformLocation(shaderProgram, "projection");
      GL_CHECK_ERROR();
      GL_CALL(glad_glUniformMatrix4fv(modelLoc, 1, GL_FALSE,
                                      glm::value_ptr(model)));
      GL_CALL(
          glad_glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view)));
      GL_CALL(glad_glUniformMatrix4fv(projLoc, 1, GL_FALSE,
                                      glm::value_ptr(projection)));

      // draw call
      GL_CALL(glDrawArrays(GL_TRIANGLES, 0, 3));
    }
  }

  GL_CALL(glBindVertexArray(0));
};

// shader compilation helper
unsigned int OpenGLRenderer::compileShader(const char *source, GLenum type) {
  unsigned int shader = glCreateShader(type);
  GL_CHECK_ERROR();
  GL_CALL(glShaderSource(shader, 1, &source, nullptr));
  GL_CALL(glCompileShader(shader));

  int success;
  GL_CALL(glGetShaderiv(shader, GL_COMPILE_STATUS, &success));
  if (!success) {
    char infoLog[EngineConstants::Graphics::SHADER_LOG_BUFFER_SIZE];
    GL_CALL(glGetShaderInfoLog(
        shader, EngineConstants::Graphics::SHADER_LOG_BUFFER_SIZE, nullptr,
        infoLog));
    std::cerr << "Shader compilation failed: \n" << infoLog << std::endl;
  }
  return shader;
};

void OpenGLRenderer::setupBuffers() {
  // Create VAO and VBO once during initialization
  GL_CALL(glGenVertexArrays(1, &VAO));
  GL_CALL(glGenBuffers(1, &VBO));

  GL_CALL(glBindVertexArray(VAO));
  GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, VBO));

  // Set up vertex attributes (position and color)
  GL_CALL(glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE,
                                EngineConstants::Graphics::VERTEX_STRIDE_SIZE *
                                    sizeof(float),
                                (void *)0));
  GL_CALL(glEnableVertexAttribArray(0));
  GL_CALL(glVertexAttribPointer(
      1, 3, GL_FLOAT, GL_FALSE,
      EngineConstants::Graphics::VERTEX_STRIDE_SIZE * sizeof(float),
      (void *)(EngineConstants::Graphics::COLOR_ATTRIBUTE_OFFSET *
               sizeof(float))));
  GL_CALL(glEnableVertexAttribArray(1));

  GL_CALL(glBindVertexArray(0));
}

void OpenGLRenderer::onUnload() {
  GL_CALL(glBindVertexArray(0));
  GL_CALL(glUseProgram(0));
  // Clean up is now handled in destructor
  m_initialized = false;
};
