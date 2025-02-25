#include "../include/CameraController.h"
#include <memory>

CameraController::CameraController(std::shared_ptr<Camera> camera,
                                   std::shared_ptr<IInputController> input)
    : m_camera(camera), m_inputController(input){};

std::shared_ptr<Camera> CameraController::camera() { return m_camera; };
std::shared_ptr<IInputController> CameraController::inputController() {
  return m_inputController;
};

void CameraController::registerControls() {
  // this is where you connect the input controller to the camera behavior
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
      {InputType::MouseMove, std::pair<float, float>{}},
      [this](float x, float y) { m_camera->rotate(x, y); });
};

// void CameraController::processAction(T action) {
// this is where you process actions

//   switch (action) {
//   case 1:
//     break;
//   default:
//     break;
//   }
// };

void CameraController::handleEvent(const InputEvent &input, float x, float y,
                                   float deltaTime) {
  m_inputController->handleEvent(input, x, y, deltaTime);
};
