#include "../include/Camera.h"
#include <glm/ext/matrix_clip_space.hpp>
#include <glm/ext/vector_float3.hpp>
#include <glm/geometric.hpp>
#include <iostream>

Camera::Camera(glm::vec3 startPosition)
    : position(startPosition), front(glm::vec3(0.0f, 0.0f, -1.0f)),
      up(glm::vec3(0.0f, 1.0f, 0.0f)), yaw(-90.0f), pitch(0.0f), fov(45.0f){};

// where the camera is looking
glm::mat4 Camera::getViewMatrix() const {
  return glm::lookAt(position, position + front, up);
};

// perspective matrix
glm::mat4 Camera::getProjectionMatrix(float aspectRatio) const {
  return glm::perspective(glm::radians(fov), aspectRatio, 0.1f, 100.0f);
};

// TODO: move this into scene action processing
void Camera::processKeyboard(char direction, float deltaTime) {
  std::cout << "in process keyboard" << std::endl;
  float speed = 2.5f * deltaTime;
  std::cout << "speed: " << speed << std::endl;
  if (direction == 'W') {
    std::cout << "W" << std::endl;
    position += speed * front;
  }
  if (direction == 'S') {
    std::cout << "S" << std::endl;
    position -= speed * front;
  }
  if (direction == 'A') {
    std::cout << "A" << std::endl;
    position -= glm::normalize(glm::cross(front, up)) * speed;
  }
  if (direction == 'D') {
    std::cout << "D" << std::endl;
    position += glm::normalize(glm::cross(front, up)) * speed;
  }
};

// TODO: move this into scene action processing
void Camera::processMouse(float xOffset, float yOffset) {
  float sensitivity = 0.05f;
  yaw += xOffset * sensitivity;
  pitch += yOffset * sensitivity;

  // limit pitch angle to avoid flipping
  if (pitch > 89.0f)
    pitch = 89.0f;
  if (pitch < -89.0f)
    pitch = -89.0f;

  // update camera direction
  glm::vec3 direction;
  direction.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
  direction.y = sin(glm::radians(pitch));
  direction.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
  front = glm::normalize(direction);
}
