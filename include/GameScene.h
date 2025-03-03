#pragma once
#include "BaseScene.hpp"
#include "CameraController.h"
#include "Entity.hpp"
#include "InputController.hpp"
#include <cstddef>
#include <glm/ext/vector_float3.hpp>
#include <memory>

enum class SceneActions { FORWARD, BACK, LEFT, RIGHT, MOUSE_MOVE, PAUSE };
enum class ActionType { PRESSED, RELEASED };

class GameScene : public BaseScene<SceneActions, ActionType> {
private:
  std::shared_ptr<Entity> m_player;
  bool m_paused = false;
  std::shared_ptr<CameraController> m_cameraController;
  std::shared_ptr<InputController> m_inputController;

public:
  // to avoid breaking polymorphism, dangerous
  // not all scenes will require a player pointer
  GameScene(const std::shared_ptr<Entity> &player = nullptr);
  GameScene(const std::shared_ptr<InputController> inputController,
            const std::shared_ptr<Entity> &player = nullptr);
  void init() override;
  void togglePaused();
  bool isPaused();
  void doMouseAction(float xOffset, float yOffset);
  std::shared_ptr<CameraController> cameraController();
  std::shared_ptr<InputController> inputController();
  void handleEvent(const sf::Event &event);
  void handleAction(const SceneActions &action, const ActionType &type,
                    float deltaTime = 0.0f, float xOffset = 0.0f,
                    float yOffset = 0.0f) override;

  void processInput(const InputEvent &event, float xOffset = 0.0f,
                    float yOffset = 0.0f, float deltaTime = 0.0f) override;
};
