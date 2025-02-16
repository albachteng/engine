#include "../include/GameScene.h"
#include <SFML/Window/Keyboard.hpp>
#include <iostream>

GameScene::GameScene(const std::shared_ptr<Entity> &player)
    : m_player(player){};

void GameScene::init() {
  std::cout << "Init fired in GameScene" << std::endl;
  std::cout << sf::Keyboard::Key::W << "W" << std::endl;
  std::cout << sf::Keyboard::Key::A << "A" << std::endl;
  std::cout << sf::Keyboard::Key::S << "S" << std::endl;
  std::cout << sf::Keyboard::Key::D << "D" << std::endl;
  std::cout << sf::Keyboard::Key::P << "P" << std::endl;
  registerAction(sf::Keyboard::Key::W, SceneActions::UP);
  registerAction(sf::Keyboard::Key::A, SceneActions::LEFT);
  registerAction(sf::Keyboard::Key::S, SceneActions::DOWN);
  registerAction(sf::Keyboard::Key::D, SceneActions::RIGHT);
  registerAction(sf::Keyboard::Key::P, SceneActions::PAUSE);
};

void GameScene::handleAction(const SceneActions &action,
                             const ActionType &type) {
  auto &transform = m_player->get<CTransform>();
  if (type == ActionType::PRESSED) {
    switch (action) {
    case SceneActions::UP:
      transform.vel += Vec2f{0.0f, -2.0f};
      break;
    case SceneActions::LEFT:
      transform.vel += Vec2f{-2.0f, 0.0f};
      break;
    case SceneActions::DOWN:
      transform.vel += Vec2f{0.0f, 2.0f};
      break;
    case SceneActions::RIGHT:
      transform.vel += Vec2f{2.0f, 0.0f};
      break;
    case SceneActions::PAUSE:
      std::cout << "PAUSED" << std::endl;
      break;
    default:
      break;
    }
  } else {
    switch (action) {
    case SceneActions::UP:
      break;
    case SceneActions::LEFT:
      break;
    case SceneActions::DOWN:
      break;
    case SceneActions::RIGHT:
      break;
    case SceneActions::PAUSE:
      std::cout << "PAUSED released" << std::endl;
      break;
    default:
      break;
    }
  }
};
