#include "Game.h"
#include <SFML/Window/Event.hpp>
#include <SFML/Window/Keyboard.hpp>
#include <iostream>

Game::Game(const std::string &config) {
  init(config);
  m_window.create(sf::VideoMode(1280, 720), "title");
  m_window.setFramerateLimit(60);
}; // read in config file

void Game::run() {
  spawnEnemy();
  while (m_window.isOpen() && m_running) {
    m_entityManager.update();
    // run systems
    sMovement();
    sRender();
    m_currentFrame++;
    sf::Event event;
    while (m_window.pollEvent(event)) {
      if (event.type == sf::Event::Closed)
        m_window.close();
    }
  }
};

void Game::spawnEnemy() {
  auto e = m_entityManager.addEntity("enemy");
  e->add<CShape>(50.0, 5, sf::Color::Red, sf::Color::White, 3.0);
  e->add<CTransform>(Vec2f{200.0, 200.0}, Vec2f{3.0, 3.0}, 45.0);
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

void Game::sMovement() {
  for (auto &e : m_entityManager.getEntities()) {
    auto &transform = e->get<CTransform>();
    transform.pos += transform.vel;
  }
};

void Game::sInput() {
  sf::Event event;
  while (m_window.pollEvent(event)) {
    if (event.type == sf::Event::KeyPressed) {
      switch (event.key.code) {
      case sf::Keyboard::W:
        std::cout << "W Key Released" << std::endl;
        break;
      case sf::Keyboard::A:
        std::cout << "A Key Released" << std::endl;
        break;
      case sf::Keyboard::S:
        std::cout << "S Key Released" << std::endl;
        break;
      case sf::Keyboard::D:
        std::cout << "D Key Released" << std::endl;
        break;
      case sf::Keyboard::P:
        std::cout << "P Key Released" << std::endl;
        // pause
        break;
      case sf::Keyboard::Escape:
        std::cout << "Escape Key Released" << std::endl;
        break;
      default:
        break;
      }
    }
    if (event.type == sf::Event::KeyReleased) {
      switch (event.key.code) {
      case sf::Keyboard::W:
        std::cout << "W Key Released" << std::endl;
        break;
      case sf::Keyboard::A:
        std::cout << "A Key Released" << std::endl;
        break;
      case sf::Keyboard::S:
        std::cout << "S Key Released" << std::endl;
        break;
      case sf::Keyboard::D:
        std::cout << "D Key Released" << std::endl;
        break;
      case sf::Keyboard::P:
        std::cout << "P Key Released" << std::endl;
        // pause
        break;
      case sf::Keyboard::Escape:
        std::cout << "Escape Key Released" << std::endl;
        break;
      default:
        break;
      }
    }
    if (event.type == sf::Event::MouseButtonPressed) {
      // shoot
    }
  }
};
