#pragma once
#include "BaseScene.hpp"
#include "Camera.h"
#include "Component.h"
#include "Entity.hpp"
#include "InputController.hpp"
#include "OpenGLRenderer.hpp"
#include "CollisionSystem.hpp"
#include "CollisionDetectionSystem.hpp"
#include "CollisionResolutionSystem.hpp"
#include "BoundarySystem.hpp"
#include "MovementSystem.hpp"
#include <cstddef>
#include <glm/ext/vector_float3.hpp>
#include <memory>

enum class SceneActions { FORWARD, BACK, LEFT, RIGHT, PAN, PAUSE, SCENE, MOUSE_TOGGLE, GRID_TOGGLE };

class GameScene : public BaseScene {
private:
  std::shared_ptr<Entity> m_player;
  bool m_paused = false;
  std::shared_ptr<Camera> m_camera;
  std::shared_ptr<ActionController<SceneActions>> m_actionController;
  std::unique_ptr<OpenGLRenderer> m_renderer;
  std::unique_ptr<CollisionSystem> m_collisionSystem;
  sf::RenderWindow& m_window;
  
  // New physics systems
  std::unique_ptr<CollisionDetectionSystem> m_collisionDetectionSystem;
  std::unique_ptr<CollisionResolutionSystem> m_collisionResolutionSystem;
  std::unique_ptr<BoundarySystem> m_boundarySystem;
  std::unique_ptr<MovementSystem> m_movementSystem;
  
  void spawnTriangle();
  Vec2f m_window_size;
  EntityManager m_entityManager;
  void sMovement(float deltaTime);
  
  // Mouse handling
  void handleMouseMovement(int mouseX, int mouseY, float deltaTime);
  void captureMouse();
  void releaseMouse();
  bool m_mouseCapture = false;
  sf::Vector2i m_windowCenter;
  
  // Mouse smoothing state
  float m_smoothedXOffset = 0.0f;
  float m_smoothedYOffset = 0.0f;
  
  // Grid rendering
  bool m_gridVisible = EngineConstants::UI::GRID_3D_DEFAULT_VISIBLE;
  bool m_gridCreated = false;
  void toggleGrid();
  void createGrid();
  void destroyGrid();

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
