#include "../include/GameScene.h"
#include <SFML/Window/Keyboard.hpp>
#include <iostream>
#include <memory>

GameScene::GameScene(const std::shared_ptr<InputController> inputController,
                     const std::shared_ptr<Entity> &player)
    : m_player(player), m_inputController(inputController),
      m_cameraController(std::make_shared<CameraController>(
          std::make_shared<Camera>(glm::vec3{0.0f, 0.0f, 3.0f}),
          inputController)) {

  m_cameraController->registerControls();
  // m_inputController.bindInput();
};

std::shared_ptr<CameraController> GameScene::cameraController() {
  return m_cameraController;
};

std::shared_ptr<InputController> GameScene::inputController() {
  return m_inputController;
};

void GameScene::init() {
  std::cout << "Init fired in GameScene" << std::endl;
  std::cout << sf::Keyboard::Key::W << "W" << std::endl;
  std::cout << sf::Keyboard::Key::A << "A" << std::endl;
  std::cout << sf::Keyboard::Key::S << "S" << std::endl;
  std::cout << sf::Keyboard::Key::D << "D" << std::endl;
  std::cout << sf::Keyboard::Key::P << "P" << std::endl;
  m_inputController->registerListener({InputType::Keyboard, sf::Keyboard::W},
                                      [](float) { /* TODO: */ });
  m_inputController->registerListener({InputType::Keyboard, sf::Keyboard::A},
                                      [](float) { /* TODO: */ });
  m_inputController->registerListener({InputType::Keyboard, sf::Keyboard::S},
                                      [](float) { /* TODO: */ });
  m_inputController->registerListener({InputType::Keyboard, sf::Keyboard::D},
                                      [](float) { /* TODO: */ });
  m_inputController->registerListener({InputType::Keyboard, sf::Keyboard::P},
                                      [](float) { /* TODO: */ });
};

void GameScene::processInput(const InputEvent &event, float xOffset,
                             float yOffset, float deltaTime) {
  m_inputController->handleEvent(event, xOffset, yOffset, deltaTime);
};

void GameScene::togglePaused() { m_paused = !m_paused; };

bool GameScene::isPaused() { return m_paused; };

void GameScene::handleAction(const SceneActions &action, const ActionType &type,
                             float deltaTime, float xOffset, float yOffset) {};
