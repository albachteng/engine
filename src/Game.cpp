#include "../include/Game.h"
#include "../include/SFMLRenderer.h"
#include <SFML/Graphics/Color.hpp>
#include <SFML/Window/Event.hpp>
#include <SFML/Window/Keyboard.hpp>
#include <cstdlib>
#include <glm/ext/vector_float3.hpp>
#include <iostream>
#include <memory>
#include <ostream>

void Game::init(const std::string &config) {};

Game::Game(const std::string &config) {
  init(config);
  m_window.create(sf::VideoMode(1280, 720), "sfml");
  m_window.setFramerateLimit(60);
  m_currentScene = std::make_shared<MapScene>(m_window);
  std::cout << "about to initialize scene" << std::endl;
  m_currentScene->onLoad();
}; // read in config file

std::shared_ptr<MapScene> Game::currentScene() { return m_currentScene; };

void Game::run() {
  std::cout << "running" << std::endl;
  while (m_window.isOpen() && m_running) {
    float deltaTime = m_deltaClock.restart().asSeconds();
    m_currentScene->update(deltaTime);
    sf::Event event;
    while (m_window.pollEvent(event)) {
      if (event.type == sf::Event::Closed) {
        m_window.close();
      }
      m_currentScene->sInput(event, deltaTime);
    }

    m_window.clear(sf::Color::Black);

    if (!currentScene()->isPaused()) {
      m_currentScene->sMovement(deltaTime);
      // sGravity();
      // other pausable systems
    }

    m_currentScene->render();
    m_window.display();
    m_currentFrame++;
  }
};

// TODO: move to scene
void Game::spawnTriangle() {
  for (int i = 0; i < 3; i++) {
    for (int j = 0; j < 3; j++) {
      for (int k = 0; k < 3; k++) {
        std::cout << "spawn Triangle loop" << std::endl;
        std::cout << "i, j, k: " << i << j << k << std::endl;
        auto e = m_entityManager.addEntity("triangle");
        e->add<CTransform3D>(
            glm::vec3{i * 2 * 1.0f, j * 3 * 1.0f, k * 4 * 1.0f},
            glm::vec3{0.0f}, glm::vec3{1.0f});
        e->add<CTriangle>();
        e->add<CAABB>(glm::vec3{i * 2 * 1.0f, j * 3 * 1.0f, k * 4 * 1.0f},
                      glm::vec3{i * 1.0f, j * 1.5f, k * 2.0f});
        e->add<CMovement3D>(glm::vec3{i * 1.0f, j * 1.0f, k * 1.0f},
                            glm::vec3{0.5f * j, 0.5f * i, 0.5f * k});
      }
    }
  }
};

// TODO: move to scene
void Game::sGravity() {
  for (auto e : m_entityManager.getEntities()) {
    e->get<CTransform>().vel += e->get<CGravity>().gravity;
  }
};

// TODO: move to scene
bool Game::AABBIntersect(const CAABB &a, const CAABB &b) {
  return (a.max.x > b.min.x && a.min.x < b.max.x) &&
         (a.max.y > b.min.y && a.min.y < b.max.y) &&
         (a.max.z > b.min.z && a.min.z < b.max.z);
};

// TODO: implement
void Game::loadScene() {

};

// TODO: chop up and move to scenes
void Game::sMovement(float deltaTime) {
  Vec2f size = (Vec2f)m_window.getSize();
  float height = size.y;
  float width = size.x;
  // MapNode selection updates
  for (auto &e : m_entityManager.getEntities("map node")) {
    auto cursor = m_currentScene->getCursor();
    std::cout << "movement: " << cursor.x << cursor.y << std::endl;
    auto selection = e->get<CSelection>();
    auto &shape = e->get<CShape>();

    if (cursor == selection.grid_position) {
      shape.circle.setFillColor(sf::Color::Black);
      shape.circle.setOutlineColor(sf::Color::White);
    } else {
      shape.circle.setFillColor(sf::Color::White);
      shape.circle.setOutlineColor(sf::Color::Cyan);
    }
  };

  for (auto &e : m_entityManager.getEntities()) {
    // 2D rotation
    // if (e->has<CTransform>())
    //   e->get<CTransform>().angle += 1.0f;

    // 3D rotation and movement
    // if (e->has<CTransform3D>() && e->has<CMovement3D>()) {
    //   e->get<CTransform3D>().rotation += glm::vec3{1.0f, 1.0f, 1.0f};
    //   e->get<CTransform3D>().position += e->get<CMovement3D>().vel *
    //   deltaTime; e->get<CMovement3D>().vel += e->get<CMovement3D>().acc *
    //   deltaTime;
    // }

    // 2D circle collision resolution
    // float radius = e->get<CShape>().circle.getRadius();
    // auto &transform = e->get<CTransform>();
    // transform.pos += transform.vel;
    // if (transform.pos.x > width - radius) {
    //   transform.pos.x = width - radius;
    //   transform.vel.x *= -0.9f;
    // }
    // if (transform.pos.x < radius) {
    //   transform.pos.x = radius;
    //   transform.vel.x *= -0.9f;
    // }
    // if (transform.pos.y > height - radius) {
    //   transform.pos.y = height - radius;
    //   transform.vel.y *= -0.9f;
    // }
    // if (transform.pos.y < radius) {
    //   transform.pos.y = radius;
    //   transform.vel.y *= -0.9f;
    // }

    // 3D AABB collision resolution
    // if (e->has<CAABB>() && e->has<CMovement3D>()) {
    //   std::cout << "id: " << e->id() << std::endl;
    //   auto a_move = e->get<CMovement3D>();
    //   auto a_pos = e->get<CTransform3D>();
    //   // collision with "walls"
    //   if (a_pos.position.x > 100.0f || a_pos.position.x < -100.0f) {
    //     a_move.vel.x *= -0.9f;
    //   }
    //   if (a_pos.position.y > 100.0f || a_pos.position.y < -100.0f) {
    //     a_move.vel.y *= -0.9f;
    //   }
    //   if (a_pos.position.z > 100.0f || a_pos.position.z < -100.0f) {
    //     a_move.vel.z *= -0.9f;
    //   }
    //   for (auto &b : m_entityManager.getEntities("triangle")) {
    //     // some velocity
    //     if (AABBIntersect(e->get<CAABB>(), b->get<CAABB>())) {
    //       std::cout << "hit! " << e->id() << ", " << b->id() << std::endl;
    //       a_move.vel *= -0.9f;
    //       auto b_move = b->get<CMovement3D>();
    //       b_move.vel *= -0.9f;
    //     }
    //   }
    // }
  }
};
