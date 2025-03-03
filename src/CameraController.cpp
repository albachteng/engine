#include "../include/CameraController.h"
#include <iostream>
#include <memory>

CameraController::CameraController(std::shared_ptr<Camera> camera,
                                   std::shared_ptr<IInputController> input)
    : m_camera(camera), m_inputController(input){};

std::shared_ptr<Camera> CameraController::camera() { return m_camera; };
std::shared_ptr<IInputController> CameraController::inputController() {
  return m_inputController;
};

void CameraController::registerControls() {
  std::cout << "registering camera controls" << std::endl;
  m_inputController->registerListener(
      {InputType::Keyboard, sf::Keyboard::W}, [this](float deltaTime) {
        m_camera->move(CameraMovement::FORWARD, deltaTime);
      });
  m_inputController->registerListener(
      {InputType::Keyboard, sf::Keyboard::A}, [this](float deltaTime) {
        m_camera->move(CameraMovement::LEFT, deltaTime);
      });
  m_inputController->registerListener(
      {InputType::Keyboard, sf::Keyboard::S}, [this](float deltaTime) {
        m_camera->move(CameraMovement::BACK, deltaTime);
      });
  m_inputController->registerListener(
      {InputType::Keyboard, sf::Keyboard::D}, [this](float deltaTime) {
        m_camera->move(CameraMovement::RIGHT, deltaTime);
      });
  m_inputController->registerAxisListener(
      {InputType::MouseMove, {}}, /* any pair */ [this](float x, float y) {
        std::cout << "firing axis listener: " << x << ", " << y << std::endl;
        m_camera->rotate(x, y);
      });
};

void CameraController::handleEvent(const InputEvent &input, float deltaTime) {
  m_inputController->handleEvent(input, deltaTime);
};
