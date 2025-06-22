#include "../include/Game.h"
#include "../include/Constants.hpp"
#include "../include/Logger.hpp"
#include <SFML/Graphics/Color.hpp>
#include <SFML/Window/Event.hpp>
#include <SFML/Window/Keyboard.hpp>
#include <cstdlib>
#include <functional>
#include <glm/ext/vector_float3.hpp>
#include <memory>
#include <ostream>

void Game::init(const std::string &config) {};

Game::Game(const std::string &config) {
  init(config);
  m_window.create(sf::VideoMode(EngineConstants::Display::WINDOW_WIDTH,
                                EngineConstants::Display::WINDOW_HEIGHT),
                  "sfml");
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
  LOG_INFO("Game: Scene initialization complete");
};

void Game::run() {
  LOG_INFO("Game: Starting main game loop");
  while (m_window.isOpen() && m_running) {
    float deltaTime = m_deltaClock.restart().asSeconds();
    m_sceneManager.processTransitions();
    auto currentScene = m_sceneManager.getCurrentScene();
    if (!currentScene) {
      LOG_WARN("Game: No active scene, skipping frame");
      continue;
    }

    currentScene->update(deltaTime);
    sf::Event event;
    while (m_window.pollEvent(event)) {
      if (event.type == sf::Event::Closed) {
        m_window.close();
      }
      if (event.type == sf::Event::KeyPressed) {
        LOG_DEBUG_STREAM("Game: Key pressed: " << event.key.code);
        if (event.key.code == sf::Keyboard::Key::Enter) {
          m_sceneManager.requestSceneTransition("MapScene");
          m_window.clear(sf::Color::Black);
          LOG_INFO("Game: Requested scene transition to MapScene");
        }
      }
      if (currentScene) {
        currentScene->sInput(event, deltaTime);
      }
    }

    m_window.clear(sf::Color::Black);

    if (currentScene && !currentScene->isPaused()) {
      currentScene->sMovement(deltaTime);
      // sGravity();
      // other pausable systems
    }

    if (currentScene) {
      currentScene->sRender();
      // systems that cannot pause
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
