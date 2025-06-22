#include "../include/Camera.h"
#include "../include/GameScene.h"
#include "../include/Constants.hpp"
#include "../include/Logger.hpp"
#include <glm/ext/matrix_clip_space.hpp>
#include <glm/ext/vector_float3.hpp>
#include <glm/geometric.hpp>

Camera::Camera(glm::vec3 startPosition)
    : position(startPosition), front(glm::vec3(0.0f, 0.0f, -1.0f)),
      up(glm::vec3(0.0f, 1.0f, 0.0f)), 
      yaw(EngineConstants::Camera::DEFAULT_YAW), 
      pitch(EngineConstants::Camera::DEFAULT_PITCH), 
      fov(EngineConstants::Camera::DEFAULT_FOV){};

// where the camera is looking
glm::mat4 Camera::getViewMatrix() const {
  return glm::lookAt(position, position + front, up);
};

// perspective matrix
glm::mat4 Camera::getProjectionMatrix(float aspectRatio) const {
  return glm::perspective(glm::radians(fov), aspectRatio, 
                         EngineConstants::Camera::NEAR_CLIP_PLANE, 
                         EngineConstants::Camera::FAR_CLIP_PLANE);
};

// TODO: move this into scene action processing
void Camera::move(CameraMovement movement, float deltaTime) {
  float speed = EngineConstants::Camera::MOVEMENT_SPEED * deltaTime;
  LOG_DEBUG_STREAM("Camera: Movement speed: " << speed);
  switch (movement) {
  case CameraMovement::FORWARD: {
    position += speed * front;
    break;
  }
  case CameraMovement::BACK: {
    position -= speed * front;
    break;
  }
  case CameraMovement::LEFT: {
    position -= glm::normalize(glm::cross(front, up)) * speed;
    break;
  }
  case CameraMovement::RIGHT: {
    position += glm::normalize(glm::cross(front, up)) * speed;
    break;
  }
  default:
    break;
  }
};

// TODO: move this into scene action processing
void Camera::rotate(float xOffset, float yOffset) {
  float sensitivity = EngineConstants::Camera::MOUSE_SENSITIVITY;
  yaw += xOffset * sensitivity;
  pitch += yOffset * sensitivity;

  // limit pitch angle to avoid flipping
  if (pitch > EngineConstants::Camera::MAX_PITCH_ANGLE)
    pitch = EngineConstants::Camera::MAX_PITCH_ANGLE;
  if (pitch < EngineConstants::Camera::MIN_PITCH_ANGLE)
    pitch = EngineConstants::Camera::MIN_PITCH_ANGLE;

  // update camera direction
  glm::vec3 direction;
  direction.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
  direction.y = sin(glm::radians(pitch));
  direction.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
  front = glm::normalize(direction);
}

// Raw rotation without additional sensitivity scaling (for pre-processed input)
void Camera::rotateRaw(float xOffset, float yOffset) {
  yaw += xOffset;
  pitch += yOffset;

  // limit pitch angle to avoid flipping
  if (pitch > EngineConstants::Camera::MAX_PITCH_ANGLE)
    pitch = EngineConstants::Camera::MAX_PITCH_ANGLE;
  if (pitch < EngineConstants::Camera::MIN_PITCH_ANGLE)
    pitch = EngineConstants::Camera::MIN_PITCH_ANGLE;

  // update camera direction
  glm::vec3 direction;
  direction.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
  direction.y = sin(glm::radians(pitch));
  direction.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
  front = glm::normalize(direction);
}
