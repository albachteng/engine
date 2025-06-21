#pragma once
#include "./Renderer.h"
#include "Camera.h"
#include "EntityManager.h"
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Window.hpp>
#include <glad/glad.h>
#include <memory>
#include <iostream>

// OpenGL Error Checking Macros
// These provide immediate feedback when OpenGL calls fail, making debugging much easier
#ifdef DEBUG
    #define GL_CHECK_ERROR() \
        do { \
            GLenum error = glGetError(); \
            if (error != GL_NO_ERROR) { \
                std::cerr << "OpenGL Error: 0x" << std::hex << error << std::dec \
                         << " (" << getGLErrorString(error) << ")" \
                         << " at " << __FILE__ << ":" << __LINE__ << std::endl; \
            } \
        } while(0)
#else
    #define GL_CHECK_ERROR() // No-op in release builds for zero performance cost
#endif

// Convenience macro for OpenGL function calls with automatic error checking
#define GL_CALL(call) \
    do { \
        call; \
        GL_CHECK_ERROR(); \
    } while(0)

// Helper function to convert OpenGL error codes to readable strings
inline const char* getGLErrorString(GLenum error) {
    switch(error) {
        case GL_NO_ERROR:          return "GL_NO_ERROR";
        case GL_INVALID_ENUM:      return "GL_INVALID_ENUM";
        case GL_INVALID_VALUE:     return "GL_INVALID_VALUE";
        case GL_INVALID_OPERATION: return "GL_INVALID_OPERATION";
        case GL_OUT_OF_MEMORY:     return "GL_OUT_OF_MEMORY";
        case GL_STACK_OVERFLOW:    return "GL_STACK_OVERFLOW";
        case GL_STACK_UNDERFLOW:   return "GL_STACK_UNDERFLOW";
        default:                   return "UNKNOWN_ERROR";
    }
}

class OpenGLRenderer : public Renderer {
public:
  OpenGLRenderer(std::shared_ptr<Camera> camera, sf::RenderWindow &window);
  ~OpenGLRenderer();
  std::shared_ptr<Camera> m_camera;

  void init() override;
  void render() override;
  void render(const EntityVec &entities) override;
  std::shared_ptr<Camera> camera();
  void onUnload();

private:
  sf::RenderWindow &m_window;
  unsigned int VAO, VBO, shaderProgram;
  bool m_initialized;
  unsigned int compileShader(const char *source, GLenum type);
  void setupBuffers();
};
