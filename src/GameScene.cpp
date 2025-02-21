#include "../include/GameScene.h"
#include <SFML/Window/Keyboard.hpp>
#include <iostream>
#include <memory>

GameScene::GameScene(const std::shared_ptr<Entity> &player)
    : m_player(player), m_cameraController(std::make_shared<Camera>(
                            glm::vec3{0.0f, 0.0f, 3.0f})){};

CameraController GameScene::cameraController() { return m_cameraController; };

void GameScene::init() {
  std::cout << "Init fired in GameScene" << std::endl;
  std::cout << sf::Keyboard::Key::W << "W" << std::endl;
  std::cout << sf::Keyboard::Key::A << "A" << std::endl;
  std::cout << sf::Keyboard::Key::S << "S" << std::endl;
  std::cout << sf::Keyboard::Key::D << "D" << std::endl;
  std::cout << sf::Keyboard::Key::P << "P" << std::endl;
  registerAction(static_cast<int>(sf::Keyboard::W), SceneActions::FORWARD);
  registerAction(static_cast<int>(sf::Keyboard::A), SceneActions::LEFT);
  registerAction(static_cast<int>(sf::Keyboard::S), SceneActions::BACK);
  registerAction(static_cast<int>(sf::Keyboard::D), SceneActions::RIGHT);
  registerAction(static_cast<int>(sf::Keyboard::P), SceneActions::PAUSE);
};

void GameScene::handleAction(const SceneActions &action, const ActionType &type,
                             float deltaTime, float xOffset, float yOffset) {
  if (type == ActionType::PRESSED) {
    switch (action) {
    case SceneActions::FORWARD:
      m_cameraController.handleKeyboard(CameraMovement::FORWARD, deltaTime);
      break;
    case SceneActions::LEFT:
      m_cameraController.handleKeyboard(CameraMovement::LEFT, deltaTime);
      break;
    case SceneActions::BACK:
      m_cameraController.handleKeyboard(CameraMovement::BACK, deltaTime);
      break;
    case SceneActions::RIGHT:
      m_cameraController.handleKeyboard(CameraMovement::RIGHT, deltaTime);
      break;
    case SceneActions::PAUSE:
      togglePaused();
      break;
    case SceneActions::MOUSE_MOVE:
      m_cameraController.handleMouse(xOffset, yOffset);
      break;
    default:
      break;
    }
  }
};

void GameScene::doMouseAction(float xOffset, float yOffset) {
  m_cameraController.handleMouse(xOffset, yOffset);
}

// void GameScene::handleAction(const SceneActions &action,
// const ActionType &type) {
// auto &transform = m_player->get<CTransform>();
// if (type == ActionType::PRESSED) {
// switch (action) {
// case SceneActions::UP:
// transform.vel += Vec2f{0.0f, -2.0f};
// break;
// case SceneActions::LEFT:
// transform.vel += Vec2f{-2.0f, 0.0f};
// break;
// case SceneActions::DOWN:
// transform.vel += Vec2f{0.0f, 2.0f};
// break;
// case SceneActions::RIGHT:
// transform.vel += Vec2f{2.0f, 0.0f};
// break;
// case SceneActions::PAUSE:
// std::cout << "PAUSED" << std::endl;
// togglePaused();
// break;
// default:
// break;
// }
// } else {
// switch (action) {
// case SceneActions::UP:
// break;
// case SceneActions::LEFT:
// break;
// case SceneActions::DOWN:
// break;
// case SceneActions::RIGHT:
// break;
// case SceneActions::PAUSE:
// std::cout << "PAUSED released" << std::endl;
// break;
// default:
// break;
// }
// }
// };

void GameScene::togglePaused() { m_paused = !m_paused; };

bool GameScene::isPaused() { return m_paused; };
