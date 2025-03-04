#include "../include/GameScene.h"
#include <SFML/Window/Keyboard.hpp>
#include <iostream>
#include <memory>
#include <unordered_map>

GameScene::GameScene(
    const std::shared_ptr<ActionController<SceneActions>> inputController,
    const std::shared_ptr<Entity> &player)
    : m_player(player), m_actionController(inputController),
      m_camera(std::make_shared<Camera>(glm::vec3{0.0f, 0.0f, 3.0f})){};

std::shared_ptr<Camera> GameScene::camera() { return m_camera; };

std::shared_ptr<ActionController<SceneActions>> GameScene::actionController() {
  return m_actionController;
};

void GameScene::init() {
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
  m_actionController->registerAxisListener(
      SceneActions::PAN, [this](float x, float y) {
        std::cout << "firing axis listener: " << x << ", " << y << std::endl;
        m_camera->rotate(x, y);
      });
  // m_inputController->registerListener({InputType::Keyboard, sf::Keyboard::W},
  //                                     [](float) { /* TODO: */ });
  // m_inputController->registerListener({InputType::Keyboard, sf::Keyboard::A},
  //                                     [](float) { /* TODO: */ });
  // m_inputController->registerListener({InputType::Keyboard, sf::Keyboard::S},
  //                                     [](float) { /* TODO: */ });
  // m_inputController->registerListener({InputType::Keyboard, sf::Keyboard::D},
  //                                     [](float) { /* TODO: */ });
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
