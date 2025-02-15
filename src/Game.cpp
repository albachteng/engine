#include "../include/Game.h"
#include "../include/GameScene.h"
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
}; // read in config file

std::shared_ptr<GameScene> Game::currentScene() { return m_currentScene; };

void Game::run() {
  spawnPlayer();
  while (m_window.isOpen() && m_running) {
    m_entityManager.update();
    std::shared_ptr<Entity> player =
        m_entityManager.getEntities("player").front();
    // std::cout << player->has<CInput>() << std::endl;
    // run systems
    sGravity();
    sMovement();
    sRender();
    m_currentFrame++;
    sf::Event event;
    while (m_window.pollEvent(event)) {
      if (event.type == sf::Event::Closed)
        m_window.close();
      sInput(event);
    }
  }
};

void Game::spawnPlayer() {
  auto e = m_entityManager.addEntity("player");
  e->add<CShape>(50.0, 5, sf::Color::Red, sf::Color::White, 3.0);
  e->add<CTransform>(Vec2f{200.0, 200.0}, Vec2f{3.0, 3.0}, 45.0);
  e->add<CInput>();
  e->add<CGravity>();
};

void Game::sRender() {
  m_window.clear(sf::Color::Black);
  for (auto e : m_entityManager.getEntities()) {
    e->get<CShape>().circle.setPosition(e->get<CTransform>().pos);
    e->get<CTransform>().angle += 1.0f;
    e->get<CShape>().circle.setRotation(e->get<CTransform>().angle);
    m_window.draw(e->get<CShape>().circle);
  };
  m_window.display();
};

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
    float radius = e->get<CShape>().circle.getRadius();
    auto &transform = e->get<CTransform>();
    transform.pos += transform.vel;
    if (e->has<CInput>()) {
      CInput &input = e->get<CInput>();
      if (input.up) {
        transform.vel += Vec2f{0.0f, -2.0f};
      }
      if (input.down) {
        transform.vel += Vec2f{0.0f, 2.0f};
      }
      if (input.left) {
        transform.vel += Vec2f{-2.0f, 0.0f};
      }
      if (input.right) {
        transform.vel += Vec2f{2.0f, 0.0f};
      }
    }
    if (transform.pos.x > width - radius) {
      // move back, reverse vel
      transform.pos.x = width - radius;
      transform.vel.x *= -1.0f;
    }
    if (transform.pos.x < radius) {
      // move back, reverse vel
      transform.pos.x = radius;
      transform.vel.x *= -1.0f;
    }
    if (transform.pos.y > height - radius) {
      // move back, reverse vel
      transform.pos.y = height - radius;
      transform.vel.y *= -1.0f;
    }
    if (transform.pos.y < radius) {
      // move back, reverse vel
      transform.pos.y = radius;
      transform.vel.y *= -1.0f;
    }
  }
};

void Game::sInput(sf::Event event) {
  auto players = m_entityManager.getEntities("player");

  if (players.empty()) {
    std::cerr << "No player entity found!" << std::endl;
    return;
  }

  std::shared_ptr<Entity> player = players.front();

  if (!player->has<CInput>()) {
    std::cerr << "Player entity is missing CInput component!" << std::endl;
    return;
  }

  CInput &input = player->get<CInput>();

  if (event.type == sf::Event::KeyPressed ||
      event.type == sf::Event::KeyReleased) {
    std::cout << "Key Pressed: " << event.key.code << std::endl;

    if (currentScene()->getActionMap().find(event.key.code) ==
        currentScene()->getActionMap().end()) {
      return;
    }

    const std::string actionType =
        (event.type == sf::Event::KeyPressed) ? "START" : "END";

    SceneActions action(currentScene()->getActionMap().at(event.key.code));
    currentScene()->doAction(action);
  }
};
