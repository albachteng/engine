#pragma once
#include "../include/EntityManager.h"
#include "../include/GameScene.h"
#include "Component.h"
#include "InputEvent.hpp"
#include "Renderer.h"
#include <SFML/Graphics.hpp>
#include <memory>
#include <optional>

struct Config {};

class Game {
private:
  sf::RenderWindow m_window;
  EntityManager m_entityManager;
  sf::Font m_font;
  sf::Text m_text;
  Config m_config;
  sf::Clock m_deltaClock;
  int m_currentFrame = 0;
  bool m_running = true;
  std::shared_ptr<GameScene> m_currentScene;
  Renderer *m_renderer;

  void init(const std::string &path);
  std::shared_ptr<Entity> spawnPlayer();
  std::shared_ptr<GameScene> currentScene();
  void spawnTriangle();

public:
  Game(const std::string &config);
  void run();
  void sMovement(float deltaTime = 0);
  void sRender();
  void sCollision();
  void sInput(sf::Event event, float deltaTime);
  void sGravity();
  std::optional<InputEvent> convertToInputEvent(const sf::Event &event);
  bool AABBIntersect(const CAABB &a, const CAABB &b);
};
