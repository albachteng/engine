#include "../include/CameraController.h"
#include <memory>

CameraController::CameraController(std::shared_ptr<Camera> camera)
    : m_camera(camera){};

std::shared_ptr<Camera> CameraController::camera() { return m_camera; };

void CameraController::handleKeyboard(CameraMovement movement,
                                      float deltaTime) {
  m_camera->processKeyboard(movement, deltaTime);
};

void CameraController::handleMouse(float xOffset, float yOffset) {
  m_camera->processMouse(xOffset, yOffset);
};
