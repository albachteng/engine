#pragma once
#include "../include/EntityManager.h"
#include "../include/GameScene.h"
#include "../include/MapScene.h"
#include "../include/VoronoiMapScene.h"
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
  Config m_config;
  bool m_running = true;
  SceneManager m_sceneManager;
  int m_currentFrame = 0;
  sf::Clock m_deltaClock;
  sf::Font m_font;
  sf::RenderWindow m_window;
  sf::Text m_text;

  void init(const std::string &path);
  void spawnTriangle();

public:
  Game(const std::string &config);
  void run();
};
