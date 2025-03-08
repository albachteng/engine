#pragma once
#include "BaseScene.hpp"
#include "Camera.h"
#include "Entity.hpp"
#include "InputController.hpp"
#include <cstddef>
#include <glm/ext/vector_float3.hpp>
#include <memory>

enum class SceneActions { FORWARD, BACK, LEFT, RIGHT, PAN, PAUSE };

class GameScene : public BaseScene<SceneActions> {
private:
  std::shared_ptr<Entity> m_player;
  bool m_paused = false;
  std::shared_ptr<Camera> m_camera;
  std::shared_ptr<ActionController<SceneActions>> m_actionController;

public:
  std::unordered_map<InputEvent, SceneActions> m_inputMap;
  // not all scenes will require a player pointer
  GameScene(const std::shared_ptr<Entity> &player = nullptr);
  GameScene(
      const std::shared_ptr<ActionController<SceneActions>> actionController,
      const std::shared_ptr<Entity> &player = nullptr);
  void init() override;
  void togglePaused();
  bool isPaused();
  std::shared_ptr<Camera> camera();
  std::shared_ptr<ActionController<SceneActions>> actionController();

  void processInput(const InputEvent &event, float deltaTime = 0.0f) override;
};
