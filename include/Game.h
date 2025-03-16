#pragma once
#include "../include/EntityManager.h"
#include "../include/GameScene.h"
#include "../include/MapScene.h"
#include "Component.h"
#include "InputEvent.hpp"
#include "Renderer.h"
#include "SceneManager.hpp"
#include <SFML/Graphics.hpp>
#include <memory>
#include <optional>

struct Config {};

class Game {
private:
  sf::RenderWindow m_window;
  sf::Font m_font;
  sf::Text m_text;
  Config m_config;
  sf::Clock m_deltaClock;
  int m_currentFrame = 0;
  bool m_running = true;
  SceneManager m_sceneManager;

  void init(const std::string &path);
  void spawnTriangle();

public:
  Game(const std::string &config);
  void run();
  void loadScene();
};
