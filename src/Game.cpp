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
  auto inputController = std::make_shared<ActionController<MapActions>>();
  std::cout << "created inputController" << std::endl;
  m_currentScene = std::make_shared<MapScene>(inputController, spawnPlayer());
  std::cout << "about to initialize scene" << std::endl;
  m_currentScene->onLoad();
  m_renderer = new SFMLRenderer(m_window);
}; // read in config file

std::shared_ptr<MapScene> Game::currentScene() { return m_currentScene; };

void Game::run() {
  std::cout << "running" << std::endl;
  spawnMapNodes();
  std::cout << "spawned map nodes" << std::endl;
  // spawnTriangle();
  while (m_window.isOpen() && m_running) {
    m_entityManager.update();
    sf::Event event;
    float deltaTime = m_deltaClock.restart().asSeconds();
    while (m_window.pollEvent(event)) {
      if (event.type == sf::Event::Closed) {
        m_window.close();
      }
      sInput(event, deltaTime);
    }

    m_window.clear(sf::Color::Black);

    if (!currentScene()->isPaused()) {
      sMovement(/*deltaTime*/);
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
  // e->add<CShape>(50.0, 5, sf::Color::Red, sf::Color::White, 3.0);
  // e->add<CTransform>(Vec2f{200.0, 200.0}, Vec2f{3.0, 3.0}, 45.0);
  e->add<CInput>();
  // e->add<CGravity>();
  return e;
};

void Game::spawnMapNodes() {
  std::cout << "entering spawnMapNodes" << std::endl;
  int cols = 10;
  int rows = 10;
  Vec2f window_size = (Vec2f)m_window.getSize();
  float node_height = window_size.y / cols;
  float node_width = window_size.x / rows;
  for (int i = 0; i < cols; i++) {
    for (int j = 0; j < rows; j++) {

      std::cout << "spawn map nodes loop: " << i << ", " << j << std::endl;
      auto e = m_entityManager.addEntity("map node");
      bool isSelected = i == 0 && j == 0;
      e->add<CShape>(node_height * .4f, 4,
                     isSelected ? sf::Color::Black : sf::Color::White,
                     isSelected ? sf::Color::White : sf::Color::Cyan, 3.0f);
      float x = i * node_width + node_width * 0.5f;
      float y = j * node_height + node_height * 0.5f;
      e->add<CTransform>(Vec2f{x, y}, Vec2f{0.0f, 0.0f}, 45.0f);
      e->add<CSelection>(Vec2i{i, j});
    }
  }
};

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

void Game::sRender() { m_renderer->render(m_entityManager.getEntities()); };

void Game::sGravity() {
  for (auto e : m_entityManager.getEntities()) {
    e->get<CTransform>().vel += e->get<CGravity>().gravity;
  }
};

bool Game::AABBIntersect(const CAABB &a, const CAABB &b) {
  return (a.max.x > b.min.x && a.min.x < b.max.x) &&
         (a.max.y > b.min.y && a.min.y < b.max.y) &&
         (a.max.z > b.min.z && a.min.z < b.max.z);
};

void Game::loadScene() {

};

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

void Game::sInput(sf::Event event, float deltaTime) {
  auto scene = currentScene();
  switch (event.type) {
  case sf::Event::KeyPressed: {
    currentScene()->processInput(
        InputEvent{InputType::Keyboard, event.key.code}, deltaTime);
    break;
  }
  // case sf::Event::KeyReleased: {
  //     currentScene()->processInput(InputEvent{InputType::KeyReleased, },
  //     ActionType::RELEASED, deltaTime);
  //   }
  //   break;
  // }
  case sf::Event::MouseMoved: {
    static float lastX = m_window.getSize().x / 2;
    static float lastY = m_window.getSize().y / 2;
    if (abs(event.mouseMove.x - lastX) < 2 &&
        abs(event.mouseMove.y - lastY) < 2)
      return; // Skip minor movements
    float xOffset = event.mouseMove.x - lastX;
    float yOffset = lastY - event.mouseMove.y; // inverted Y
    lastX = event.mouseMove.x;
    lastY = event.mouseMove.y;
    std::cout << "delta: " << deltaTime << std::endl;
    currentScene()->processInput(
        InputEvent{InputType::MouseMove,
                   std::pair<float, float>{xOffset, yOffset}},
        deltaTime);
    break;
  }
  default:
    break;
  }
};
