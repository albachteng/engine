#pragma once
#include "../include/BaseScene.h"
#include "../include/EntityManager.h"
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
  bool m_paused = false;
  bool m_running = true;

  void init(const std::string &path);
  void setPaused(bool paused);
  void spawnPlayer();
  std::shared_ptr<Scene> currentScene();

public:
  Game(const std::string &config);
  void run();
  void sMovement();
  void sRender();
  void sCollision();
  void sInput(sf::Event event);
  void sGravity();
};
