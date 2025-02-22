#pragma once
#include "BaseScene.hpp"
#include "CameraController.h"
#include "Entity.hpp"
#include <cstddef>
#include <glm/ext/vector_float3.hpp>
#include <memory>

enum class SceneActions { FORWARD, BACK, LEFT, RIGHT, MOUSE_MOVE, PAUSE };
enum class ActionType { PRESSED, RELEASED };

class GameScene : public BaseScene<SceneActions, ActionType> {
private:
  std::shared_ptr<Entity> m_player;
  bool m_paused = false;
  CameraController m_cameraController;

public:
  // to avoid breaking polymorphism, dangerous
  // not all scenes will require a player pointer
  GameScene(const std::shared_ptr<Entity> &player = nullptr);
  void init() override;
  void togglePaused();
  bool isPaused();
  void doMouseAction(float xOffset, float yOffset);
  CameraController cameraController();

protected:
  void handleAction(const SceneActions &action, const ActionType &type,
                    float deltaTime = 0.0f, float xOffset = 0.0f,
                    float yOffset = 0.0f) override;
};
