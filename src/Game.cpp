#include "../include/Game.h"
#include <SFML/Graphics/Color.hpp>
#include <SFML/Window/Event.hpp>
#include <SFML/Window/Keyboard.hpp>
#include <cstdlib>
#include <functional>
#include <glm/ext/vector_float3.hpp>
#include <iostream>
#include <memory>
#include <ostream>

void Game::init(const std::string &config) {};

Game::Game(const std::string &config) {
  init(config);
  m_window.create(sf::VideoMode(1280, 720), "sfml");
  m_window.setFramerateLimit(60);
  m_sceneManager.registerScene(
      "MapScene", std::function<std::shared_ptr<BaseScene>()>([this]() {
        return std::static_pointer_cast<BaseScene>(
            std::make_shared<MapScene>(m_window));
      }));
  m_sceneManager.registerScene(
      "GameScene", std::function<std::shared_ptr<BaseScene>()>([this]() {
        return std::static_pointer_cast<BaseScene>(
            std::make_shared<GameScene>(m_window));
      }));
  m_sceneManager.loadScene("GameScene");
  std::cout << "about to initialize scene" << std::endl;
}; // read in config file

void Game::run() {
  std::cout << "running" << std::endl;
  while (m_window.isOpen() && m_running) {
    float deltaTime = m_deltaClock.restart().asSeconds();
    m_sceneManager.getCurrentScene()->update(deltaTime);
    sf::Event event;
    while (m_window.pollEvent(event)) {
      if (event.type == sf::Event::Closed) {
        m_window.close();
      }
      if (event.type == sf::Event::KeyPressed) {
        std::cout << "key pressed: " << event.key.code << std::endl;
        if (event.key.code == sf::Keyboard::Key::Enter) {
          m_sceneManager.loadScene("MapScene");
          m_window.clear(sf::Color::Black);
          std::cout << "finished loadScene" << std::endl;
        }
      }
      m_sceneManager.getCurrentScene()->sInput(event, deltaTime);
    }

    m_window.clear(sf::Color::Black);

    if (!m_sceneManager.getCurrentScene()->isPaused()) {
      m_sceneManager.getCurrentScene()->sMovement(deltaTime);
      // sGravity();
      // other pausable systems
    }

    m_sceneManager.getCurrentScene()->sRender();
    m_window.display();
    m_currentFrame++;
  }
};

// TODO: move to scene
// void Game::sGravity() {
//   for (auto e : m_entityManager.getEntities()) {
//     e->get<CTransform>().vel += e->get<CGravity>().gravity;
//   }
// };

// TODO: implement
void Game::loadScene() {

};
