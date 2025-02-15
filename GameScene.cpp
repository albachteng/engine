#include "include/GameScene.h"

GameScene::GameScene(std::shared_ptr<Entity> player) : m_player(player){};

void GameScene::init() {
  registerAction(sf::Keyboard::Key::W, SceneActions::UP);
  registerAction(sf::Keyboard::Key::A, SceneActions::LEFT);
  registerAction(sf::Keyboard::Key::S, SceneActions::DOWN);
  registerAction(sf::Keyboard::Key::D, SceneActions::RIGHT);
  registerAction(sf::Keyboard::Key::P, SceneActions::PAUSE);
};

void GameScene::handleAction(const SceneActions &action) {
  auto &transform = m_player->get<CTransform>();
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
    break;
  }
};
