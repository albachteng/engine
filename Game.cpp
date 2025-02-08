#include "EntityManager.cpp"
#include <SFML/Graphics.hpp>
#include <iostream>

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

  void init(const std::string &path) {};
  void setPaused(bool paused) { m_paused = paused; };

public:
  Game(const std::string &config) {
    init(config);
    m_window.create(sf::VideoMode(1280, 720), "title");
    m_window.setFramerateLimit(60);
  }; // read in config file
  void run() {
    m_entityManager.update();
    // run systems
    m_currentFrame++;
  };
};
