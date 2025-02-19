#include "../include/Game.h"
#include "../include/GameScene.h"
#include "../include/OpenGLRenderer.hpp"
#include "../include/SFMLRenderer.h"
#include <SFML/Graphics/Color.hpp>
#include <SFML/Window/Event.hpp>
#include <SFML/Window/Keyboard.hpp>
#include <iostream>
#include <memory>
#include <ostream>

void Game::init(const std::string &config) {};

Game::Game(const std::string &config) {
  init(config);
  m_window.create(sf::VideoMode(1280, 720), "title");
  m_window.setFramerateLimit(60);
  m_currentScene = std::make_shared<GameScene>(spawnPlayer());
  m_currentScene->init();
  m_renderer = new OpenGLRenderer(m_currentScene->camera(), m_window);
}; // read in config file

std::shared_ptr<GameScene> Game::currentScene() { return m_currentScene; };

void Game::run() {
  while (m_window.isOpen() && m_running) {
    m_entityManager.update();
    sf::Event event;
    while (m_window.pollEvent(event)) {
      if (event.type == sf::Event::Closed) {
        m_window.close();
      }
      float deltaTime = m_deltaClock.restart().asSeconds();
      sInput(event, deltaTime);
    }

    m_window.clear(sf::Color::Black);

    if (!currentScene()->isPaused()) {
      sMovement();
      // sGravity();
      // other pausable systems
    }

    sRender();
    m_window.display();
    m_currentFrame++;
  }
};

std::shared_ptr<Entity> Game::spawnPlayer() {
  auto e = m_entityManager.addEntity("player");
  e->add<CShape>(50.0, 5, sf::Color::Red, sf::Color::White, 3.0);
  e->add<CTransform>(Vec2f{200.0, 200.0}, Vec2f{3.0, 3.0}, 45.0);
  e->add<CInput>();
  // e->add<CGravity>();
  return e;
};

void Game::sRender() { m_renderer->render(m_entityManager.getEntities()); };

void Game::sGravity() {
  for (auto e : m_entityManager.getEntities()) {
    e->get<CTransform>().vel += e->get<CGravity>().gravity;
  }
}

void Game::sMovement() {
  Vec2f size = (Vec2f)m_window.getSize();
  float height = size.y;
  float width = size.x;
  for (auto &e : m_entityManager.getEntities()) {
    e->get<CTransform>().angle += 1.0f;
    float radius = e->get<CShape>().circle.getRadius();
    auto &transform = e->get<CTransform>();
    transform.pos += transform.vel;
    if (transform.pos.x > width - radius) {
      // move back, reverse vel
      transform.pos.x = width - radius;
      transform.vel.x *= -0.9f;
    }
    if (transform.pos.x < radius) {
      // move back, reverse vel
      transform.pos.x = radius;
      transform.vel.x *= -0.9f;
    }
    if (transform.pos.y > height - radius) {
      // move back, reverse vel
      transform.pos.y = height - radius;
      transform.vel.y *= -0.9f;
    }
    if (transform.pos.y < radius) {
      // move back, reverse vel
      transform.pos.y = radius;
      transform.vel.y *= -0.9f;
    }
  }
};

void Game::sInput(sf::Event event, float deltaTime) {
  auto scene = currentScene();
  auto actionOpt = scene->getAction(event.key.code);
  switch (event.type) {
  case sf::Event::KeyPressed: {
    if (actionOpt.has_value()) {
      auto action = actionOpt.value();
      currentScene()->doAction(action, ActionType::PRESSED, deltaTime);
    }
    break;
  }
  case sf::Event::KeyReleased: {
    if (actionOpt.has_value()) {
      auto action = actionOpt.value();
      currentScene()->doAction(action, ActionType::RELEASED, deltaTime);
    }
    break;
  }
  case sf::Event::MouseMoved: {
    static float lastX = event.mouseMove.x;
    static float lastY = event.mouseMove.y;
    float xOffset = event.mouseMove.x - lastX;
    float yOffset = event.mouseMove.y - lastY;
    lastX = event.mouseMove.x;
    lastY = event.mouseMove.y;
    currentScene()->doMouseAction(xOffset, yOffset);
    break;
  }
  default:
    break;
  }
};
