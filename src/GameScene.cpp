#include "../include/GameScene.h"
#include <SFML/Window/Keyboard.hpp>
#include <iostream>
#include <memory>

GameScene::GameScene(const std::shared_ptr<Entity> &player)
    : m_player(player),
      m_camera(std::make_shared<Camera>(glm::vec3{0.0f, 0.0f, 3.0f})){};

void GameScene::init() {
  std::cout << "Init fired in GameScene" << std::endl;
  std::cout << sf::Keyboard::Key::W << "W" << std::endl;
  std::cout << sf::Keyboard::Key::A << "A" << std::endl;
  std::cout << sf::Keyboard::Key::S << "S" << std::endl;
  std::cout << sf::Keyboard::Key::D << "D" << std::endl;
  std::cout << sf::Keyboard::Key::P << "P" << std::endl;
  registerAction(static_cast<int>(sf::Keyboard::W), SceneActions::UP);
  registerAction(static_cast<int>(sf::Keyboard::A), SceneActions::LEFT);
  registerAction(static_cast<int>(sf::Keyboard::S), SceneActions::DOWN);
  registerAction(static_cast<int>(sf::Keyboard::D), SceneActions::RIGHT);
  registerAction(static_cast<int>(sf::Keyboard::P), SceneActions::PAUSE);
};

std::shared_ptr<Camera> GameScene::camera() { return m_camera; };

void GameScene::handleAction(const SceneActions &action, const ActionType &type,
                             float deltaTime) {
  if (type == ActionType::PRESSED) {
    switch (action) {
    case SceneActions::UP:
      m_camera->processKeyboard('W', deltaTime);
      break;
    case SceneActions::LEFT:
      m_camera->processKeyboard('A', deltaTime);
      break;
    case SceneActions::DOWN:
      m_camera->processKeyboard('S', deltaTime);
      break;
    case SceneActions::RIGHT:
      m_camera->processKeyboard('D', deltaTime);
      break;
    case SceneActions::PAUSE:
      m_camera->processKeyboard('P', deltaTime);
      break;
    default:
      break;
    }
  }
};

void GameScene::doMouseAction(float xOffset, float yOffset) {
  m_camera->processMouse(xOffset, yOffset);
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
