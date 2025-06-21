#include "../include/Game.h"
#include "../include/Constants.hpp"
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
  m_window.create(sf::VideoMode(EngineConstants::Display::WINDOW_WIDTH, 
                                EngineConstants::Display::WINDOW_HEIGHT), "sfml");
  m_window.setFramerateLimit(EngineConstants::Display::TARGET_FRAMERATE);
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
  // Use new thread-safe API for initial scene loading
  m_sceneManager.requestSceneTransition("GameScene");
  m_sceneManager.processTransitions(); // Process immediately for startup
  std::cout << "about to initialize scene" << std::endl;
}; // read in config file

void Game::run() {
  std::cout << "running" << std::endl;
  while (m_window.isOpen() && m_running) {
    float deltaTime = m_deltaClock.restart().asSeconds();
    
    // Process any pending scene transitions at safe point
    m_sceneManager.processTransitions();
    
    // Get current scene safely
    auto currentScene = m_sceneManager.getCurrentScene();
    if (!currentScene) {
      // No active scene - skip frame but keep running
      std::cerr << "Warning: No active scene, skipping frame" << std::endl;
      continue;
    }
    
    currentScene->update(deltaTime);
    sf::Event event;
    while (m_window.pollEvent(event)) {
      if (event.type == sf::Event::Closed) {
        m_window.close();
      }
      if (event.type == sf::Event::KeyPressed) {
        std::cout << "key pressed: " << event.key.code << std::endl;
        if (event.key.code == sf::Keyboard::Key::Enter) {
          // Use thread-safe scene transition request
          m_sceneManager.requestSceneTransition("MapScene");
          m_window.clear(sf::Color::Black);
          std::cout << "requested scene transition to MapScene" << std::endl;
        }
      }
      // Safe scene access for input handling
      if (currentScene) {
        currentScene->sInput(event, deltaTime);
      }
    }

    m_window.clear(sf::Color::Black);

    // Safe scene access for movement and rendering
    if (currentScene && !currentScene->isPaused()) {
      currentScene->sMovement(deltaTime);
      // sGravity();
      // other pausable systems
    }

    if (currentScene) {
      currentScene->sRender();
    }
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
