#include "../include/GameScene.h"
#include <SFML/Window/Keyboard.hpp>
#include <iostream>
#include <memory>
#include <unordered_map>

void GameScene::onUnload() {
  std::cout << "unloading GameScene" << std::endl;
  m_entityManager.clear();
  m_renderer->onUnload();
  m_renderer.reset();
  m_actionController->unregisterAll();
  m_actionController.reset();
  m_camera.reset();
};

void GameScene::update(float deltaTime) { m_entityManager.update(); };

void GameScene::sRender() {
  m_renderer->render(m_entityManager.getEntities());
};

GameScene::GameScene(sf::RenderWindow &window) {
  m_camera = std::make_shared<Camera>(glm::vec3{0.0f, 0.0f, 3.0f});
  m_window_size = (Vec2f)window.getSize(); // TODO: handle resize
  m_renderer = std::make_unique<OpenGLRenderer>(m_camera, window);
  m_actionController = std::make_shared<ActionController<SceneActions>>();
  spawnTriangle();
};

std::shared_ptr<Camera> GameScene::camera() { return m_camera; };

std::shared_ptr<ActionController<SceneActions>> GameScene::actionController() {
  return m_actionController;
};

void GameScene::onLoad() {
  std::cout << "Init fired in GameScene" << std::endl;
  std::cout << "registering input map" << std::endl;
  m_inputMap[InputEvent{InputType::Keyboard, sf::Keyboard::W}] =
      SceneActions::FORWARD;
  m_inputMap[InputEvent{InputType::Keyboard, sf::Keyboard::A}] =
      SceneActions::LEFT;
  m_inputMap[InputEvent{InputType::Keyboard, sf::Keyboard::S}] =
      SceneActions::BACK;
  m_inputMap[InputEvent{InputType::Keyboard, sf::Keyboard::D}] =
      SceneActions::RIGHT;
  m_inputMap[InputEvent{InputType::Keyboard, sf::Keyboard::P}] =
      SceneActions::PAUSE;
  m_inputMap[InputEvent{InputType::MouseMove, {} /* any pair */}] =
      SceneActions::PAN;
  m_inputMap[InputEvent{InputType::Keyboard, sf::Keyboard::Up}] =
      SceneActions::FORWARD;
  m_inputMap[InputEvent{InputType::Keyboard, sf::Keyboard::Left}] =
      SceneActions::LEFT;
  m_inputMap[InputEvent{InputType::Keyboard, sf::Keyboard::Down}] =
      SceneActions::BACK;
  m_inputMap[InputEvent{InputType::Keyboard, sf::Keyboard::Right}] =
      SceneActions::RIGHT;
  m_inputMap[InputEvent{InputType::Keyboard, sf::Keyboard::Enter}] =
      SceneActions::SCENE;

  std::cout << "registering listeners" << std::endl;
  m_actionController->registerListener(SceneActions::PAUSE,
                                       [this](float) { togglePaused(); });
  m_actionController->registerListener(
      SceneActions::FORWARD, [this](float deltaTime) {
        m_camera->move(CameraMovement::FORWARD, deltaTime);
      });
  m_actionController->registerListener(
      SceneActions::LEFT, [this](float deltaTime) {
        m_camera->move(CameraMovement::LEFT, deltaTime);
      });
  m_actionController->registerListener(
      SceneActions::BACK, [this](float deltaTime) {
        m_camera->move(CameraMovement::BACK, deltaTime);
      });
  m_actionController->registerListener(
      SceneActions::RIGHT, [this](float deltaTime) {
        m_camera->move(CameraMovement::RIGHT, deltaTime);
      });
  m_actionController->registerListener(SceneActions::SCENE,
                                       [this](float deltaTime) {
                                         /* TODO: swap scenes */
                                       });
  m_actionController->registerAxisListener(SceneActions::PAN,
                                           [this](float x, float y) {
                                             // std::cout << "firing axis
                                             // listener: " << x << ", " << y <<
                                             // std::endl;
                                             m_camera->rotate(x, y);
                                           });
};

void GameScene::spawnTriangle() {
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

void GameScene::sInput(sf::Event &event, float deltaTime) {
  switch (event.type) {
  case sf::Event::KeyPressed: {
    processInput(InputEvent{InputType::Keyboard, event.key.code}, deltaTime);
    break;
  }
  // case sf::Event::KeyReleased: {
  //     currentScene()->processInput(InputEvent{InputType::KeyReleased, },
  //     ActionType::RELEASED, deltaTime);
  //   }
  //   break;
  // }
  case sf::Event::MouseMoved: {
    static float lastX = m_window_size.x / 2;
    static float lastY = m_window_size.y / 2;
    if (abs(event.mouseMove.x - lastX) < 2 &&
        abs(event.mouseMove.y - lastY) < 2)
      return; // Skip minor movements
    float xOffset = event.mouseMove.x - lastX;
    float yOffset = lastY - event.mouseMove.y; // inverted Y
    lastX = event.mouseMove.x;
    lastY = event.mouseMove.y;
    // std::cout << "delta: " << deltaTime << std::endl;
    processInput(InputEvent{InputType::MouseMove,
                            std::pair<float, float>{xOffset, yOffset}},
                 deltaTime);
    break;
  }
  default:
    break;
  }
};

void GameScene::processInput(const InputEvent &input, float deltaTime) {
  auto action = m_inputMap.find(input);
  if (action != m_inputMap.end()) {
    std::pair<float, float> pair = {0.0f, 0.0f};
    if (input.type == InputType::MouseMove) {
      pair = std::get<std::pair<float, float>>(input.data);
    }
    m_actionController->handleEvent(action->second, deltaTime, pair.first,
                                    pair.second);
  }
};

void GameScene::togglePaused() { m_paused = !m_paused; };

bool GameScene::isPaused() { return m_paused; };

bool GameScene::AABBIntersect(const CAABB &a, const CAABB &b) {
  return (a.max.x > b.min.x && a.min.x < b.max.x) &&
         (a.max.y > b.min.y && a.min.y < b.max.y) &&
         (a.max.z > b.min.z && a.min.z < b.max.z);
};

void GameScene::sMovement(float deltaTime) {
  float height = m_window_size.y;
  float width = m_window_size.x;

  for (auto &e : m_entityManager.getEntities()) {
    // 2D rotation
    // if (e->has<CTransform>())
    //   e->get<CTransform>().angle += 1.0f;

    // 3D rotation and movement
    if (e->has<CTransform3D>() && e->has<CMovement3D>()) {
      e->get<CTransform3D>().rotation += glm::vec3{1.0f, 1.0f, 1.0f};
      e->get<CTransform3D>().position += e->get<CMovement3D>().vel * deltaTime;
      e->get<CMovement3D>().vel += e->get<CMovement3D>().acc * deltaTime;
    }

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
    if (e->has<CAABB>() && e->has<CMovement3D>()) {
      // std::cout << "id: " << e->id() << std::endl;
      auto a_move = e->get<CMovement3D>();
      auto a_pos = e->get<CTransform3D>();
      // collision with "walls"
      if (a_pos.position.x > 100.0f || a_pos.position.x < -100.0f) {
        a_move.vel.x *= -0.9f;
      }
      if (a_pos.position.y > 100.0f || a_pos.position.y < -100.0f) {
        a_move.vel.y *= -0.9f;
      }
      if (a_pos.position.z > 100.0f || a_pos.position.z < -100.0f) {
        a_move.vel.z *= -0.9f;
      }
      for (auto &b : m_entityManager.getEntities("triangle")) {
        // some velocity
        if (AABBIntersect(e->get<CAABB>(), b->get<CAABB>())) {
          // std::cout << "hit! " << e->id() << ", " << b->id() << std::endl;
          a_move.vel *= -0.9f;
          auto b_move = b->get<CMovement3D>();
          b_move.vel *= -0.9f;
        }
      }
    }
  }
};
