#pragma once
#include "../include/EntityManager.h"
#include "../include/GameScene.h"
#include "Renderer.h"
#include <SFML/Graphics.hpp>
#include <memory>

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

public:
  Game(const std::string &config);
  void run();
  void sMovement();
  void sRender();
  void sCollision();
  void sInput(sf::Event event);
  void sGravity();
};
