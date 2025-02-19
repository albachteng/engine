#pragma once
#include "BaseScene.hpp"
#include "Camera.h"
#include "Entity.hpp"
#include <cstddef>
#include <glm/ext/vector_float3.hpp>
#include <memory>

enum class SceneActions { UP, DOWN, LEFT, RIGHT, PAUSE };
enum class ActionType { PRESSED, RELEASED };

class GameScene : public BaseScene<SceneActions, ActionType> {
private:
  std::shared_ptr<Entity> m_player;
  bool m_paused = false;
  std::shared_ptr<Camera> m_camera;

public:
  // to avoid breaking polymorphism, dangerous
  // not all scenes will require a player pointer
  GameScene(const std::shared_ptr<Entity> &player = nullptr);
  void init() override;
  void togglePaused();
  bool isPaused();
  std::shared_ptr<Camera> camera();
  void doMouseAction(float xOffset, float yOffset);

protected:
  void handleAction(const SceneActions &action, const ActionType &type,
                    float deltaTime) override;
};
