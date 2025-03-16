#pragma once
#include "BaseScene.hpp"
#include "Camera.h"
#include "Component.h"
#include "Entity.hpp"
#include "InputController.hpp"
#include "OpenGLRenderer.hpp"
#include <cstddef>
#include <glm/ext/vector_float3.hpp>
#include <memory>

enum class SceneActions { FORWARD, BACK, LEFT, RIGHT, PAN, PAUSE };

class GameScene : public BaseScene {
private:
  std::shared_ptr<Entity> m_player;
  bool m_paused = false;
  std::shared_ptr<Camera> m_camera;
  std::shared_ptr<ActionController<SceneActions>> m_actionController;
  std::unique_ptr<OpenGLRenderer> m_renderer;
  void spawnTriangle();
  Vec2f m_window_size;
  EntityManager m_entityManager;
  void sMovement(float deltaTime);

public:
  bool AABBIntersect(const CAABB &a, const CAABB &b);
  void onUnload() override;
  void update(float deltaTime) override;
  void sRender() override;
  void onLoad() override;
  void processInput(const InputEvent &event, float deltaTime = 0.0f) override;
  void sInput(sf::Event &event, float deltaTime) override;

  std::unordered_map<InputEvent, SceneActions> m_inputMap;
  // not all scenes will require a player pointer
  explicit GameScene(sf::RenderWindow &window);

  std::shared_ptr<Camera> camera();
  std::shared_ptr<ActionController<SceneActions>> actionController();

  void togglePaused();
  bool isPaused();
};
