#include "../include/GameScene.h"
#include "../include/Constants.hpp"
#include "../include/Logger.hpp"
#include <SFML/Window/Keyboard.hpp>
#include <memory>
#include <unordered_map>
#include <cmath>
#include <algorithm>

void GameScene::onUnload() {
  LOG_INFO("GameScene: Unloading scene and cleaning up resources");
  m_entityManager.clear();
  m_collisionSystem->clear();
  m_renderer->onUnload();
  m_renderer.reset();
  m_actionController->unregisterAll();
  m_actionController.reset();
  m_camera.reset();
  
  // Clean up new physics systems
  m_collisionDetectionSystem.reset();
  m_collisionResolutionSystem.reset();
  m_boundarySystem.reset();
  m_movementSystem.reset();
};

void GameScene::update(float deltaTime) { m_entityManager.update(); };

void GameScene::sRender() {
  m_renderer->render(m_entityManager.getEntities());
};

GameScene::GameScene(sf::RenderWindow &window) : m_window(window) {
  m_camera = std::make_shared<Camera>(glm::vec3{
      EngineConstants::Camera::START_X, EngineConstants::Camera::START_Y,
      EngineConstants::Camera::START_Z});
  m_window_size = (Vec2f)window.getSize(); // TODO: handle resize
  m_windowCenter = sf::Vector2i(m_window_size.x / 2, m_window_size.y / 2);
  m_renderer = std::make_unique<OpenGLRenderer>(m_camera, window);
  m_actionController = std::make_shared<ActionController<SceneActions>>();

  // Initialize collision system with uniform grid
  m_collisionSystem = std::make_unique<CollisionSystem>(
      PartitionType::UNIFORM_GRID,
      glm::vec3(EngineConstants::World::MIN_BOUND,
                EngineConstants::World::MIN_BOUND,
                EngineConstants::World::MIN_BOUND),
      glm::vec3(EngineConstants::World::MAX_BOUND,
                EngineConstants::World::MAX_BOUND,
                EngineConstants::World::MAX_BOUND),
      EngineConstants::SpatialPartition::DEFAULT_CELL_SIZE);

  // Initialize new physics systems
  m_collisionDetectionSystem = std::make_unique<CollisionDetectionSystem>();
  m_collisionResolutionSystem = std::make_unique<CollisionResolutionSystem>();
  m_movementSystem = std::make_unique<MovementSystem>();
  
  // Initialize boundary system with world bounds
  BoundaryConstraint worldBounds(
      glm::vec3(EngineConstants::World::MIN_BOUND),
      glm::vec3(EngineConstants::World::MAX_BOUND),
      BoundaryAction::BOUNCE,
      -EngineConstants::World::COLLISION_DAMPING_FACTOR
  );
  m_boundarySystem = std::make_unique<BoundarySystem>(worldBounds);
  
  // Configure collision response for triangles
  m_collisionResolutionSystem->setDefaultResponse(CollisionResponseType::DAMPED, 0.9f);
  m_collisionResolutionSystem->setEntityResponse(EntityTag::TRIANGLE, 
      CollisionResponse(CollisionResponseType::DAMPED, 0.9f, 0.1f));

  spawnTriangle();
};

std::shared_ptr<Camera> GameScene::camera() { return m_camera; };

std::shared_ptr<ActionController<SceneActions>> GameScene::actionController() {
  return m_actionController;
};

void GameScene::onLoad() {
  LOG_INFO("GameScene: Initializing scene");
  LOG_DEBUG("GameScene: Registering input mappings");
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
  m_inputMap[InputEvent{InputType::Keyboard, sf::Keyboard::Escape}] =
      SceneActions::MOUSE_TOGGLE;
  m_inputMap[InputEvent{InputType::Keyboard, sf::Keyboard::G}] =
      SceneActions::GRID_TOGGLE;

  LOG_DEBUG("GameScene: Registering input listeners");
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
  m_actionController->registerListener(SceneActions::MOUSE_TOGGLE,
                                       [this](float) { 
                                         if (m_mouseCapture) releaseMouse(); 
                                         else captureMouse(); 
                                       });
  m_actionController->registerListener(SceneActions::GRID_TOGGLE,
                                       [this](float) { toggleGrid(); });
  m_actionController->registerAxisListener(
      SceneActions::PAN, [this](float x, float y) { m_camera->rotateRaw(x, y); });
  
  // Enable mouse capture by default
  captureMouse();
};

void GameScene::spawnTriangle() {
  for (int i = 0; i < EngineConstants::World::ENTITY_GRID_SIZE; i++) {
    for (int j = 0; j < EngineConstants::World::ENTITY_GRID_SIZE; j++) {
      for (int k = 0; k < EngineConstants::World::ENTITY_GRID_SIZE; k++) {
        LOG_DEBUG_STREAM("GameScene: Spawning triangle at position "
                         << i << ", " << j << ", " << k);
        auto e = m_entityManager.addEntity(EntityTag::TRIANGLE);
        e->add<CTransform3D>(
            glm::vec3{i * EngineConstants::World::ENTITY_SPACING_X,
                      j * EngineConstants::World::ENTITY_SPACING_Y,
                      k * EngineConstants::World::ENTITY_SPACING_Z},
            glm::vec3{0.0f}, glm::vec3{1.0f});
        e->add<CTriangle>();
        e->add<CAABB>(glm::vec3{0.0f, 0.0f, 0.0f}, // Center relative to entity position
                      glm::vec3{0.5f, 0.5f, 0.5f}); // Half-extents for triangle bounding box
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
    handleMouseMovement(event.mouseMove.x, event.mouseMove.y, deltaTime);
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
  if (m_paused) {
    return; // Skip physics update when paused
  }
  
  LOG_DEBUG("GameScene: Running physics update with new systems");
  
  // 1. Update movement (position from velocity, velocity from acceleration)
  m_movementSystem->updateMovement(m_entityManager, deltaTime);
  
  // 2. Detect collisions between entities
  auto collisions = m_collisionDetectionSystem->detectCollisions(m_entityManager);
  
  // 3. Resolve collisions (apply physics response)
  m_collisionResolutionSystem->resolveCollisions(collisions);
  
  // 4. Enforce world boundaries
  m_boundarySystem->enforceBoundaries(m_entityManager);
  
  // 5. Update legacy collision system for compatibility (if needed by other systems)
  m_collisionSystem->updateEntities(m_entityManager);
};

void GameScene::handleMouseMovement(int mouseX, int mouseY, float deltaTime) {
  if (!m_mouseCapture) return;
  
  // Calculate relative movement from center
  float rawXOffset = mouseX - m_windowCenter.x;
  float rawYOffset = m_windowCenter.y - mouseY; // Inverted Y
  
  // Only process if movement exceeds threshold
  if (abs(rawXOffset) < EngineConstants::Input::MOUSE_MOVEMENT_THRESHOLD &&
      abs(rawYOffset) < EngineConstants::Input::MOUSE_MOVEMENT_THRESHOLD) {
    return;
  }
  
  // Clamp delta to prevent wild rotation from large movements
  float clampedXOffset = std::max(-EngineConstants::Input::MOUSE_MAX_DELTA, 
                                  std::min(EngineConstants::Input::MOUSE_MAX_DELTA, rawXOffset));
  float clampedYOffset = std::max(-EngineConstants::Input::MOUSE_MAX_DELTA, 
                                  std::min(EngineConstants::Input::MOUSE_MAX_DELTA, rawYOffset));
  
  // Apply sensitivity scaling
  float scaledXOffset = clampedXOffset * EngineConstants::Input::MOUSE_SENSITIVITY_SCALE * 
                        EngineConstants::Input::MOUSE_SENSITIVITY_X;
  float scaledYOffset = clampedYOffset * EngineConstants::Input::MOUSE_SENSITIVITY_SCALE * 
                        EngineConstants::Input::MOUSE_SENSITIVITY_Y;
  
  // Apply acceleration (power curve for fine control)
  if (EngineConstants::Input::MOUSE_ACCELERATION != 1.0f) {
    float xSign = scaledXOffset >= 0 ? 1.0f : -1.0f;
    float ySign = scaledYOffset >= 0 ? 1.0f : -1.0f;
    scaledXOffset = xSign * pow(abs(scaledXOffset), EngineConstants::Input::MOUSE_ACCELERATION);
    scaledYOffset = ySign * pow(abs(scaledYOffset), EngineConstants::Input::MOUSE_ACCELERATION);
  }
  
  // Apply smoothing if enabled
  float finalXOffset, finalYOffset;
  if (EngineConstants::Input::ENABLE_MOUSE_SMOOTHING) {
    float smoothing = EngineConstants::Input::MOUSE_SMOOTHING;
    m_smoothedXOffset = m_smoothedXOffset * smoothing + scaledXOffset * (1.0f - smoothing);
    m_smoothedYOffset = m_smoothedYOffset * smoothing + scaledYOffset * (1.0f - smoothing);
    finalXOffset = m_smoothedXOffset;
    finalYOffset = m_smoothedYOffset;
  } else {
    finalXOffset = scaledXOffset;
    finalYOffset = scaledYOffset;
  }
  
  // Process the mouse movement
  processInput(InputEvent{InputType::MouseMove,
                          std::pair<float, float>{finalXOffset, finalYOffset}},
               deltaTime);
  
  // Reset mouse to center of window for continuous movement
  sf::Mouse::setPosition(m_windowCenter, m_window);
}

void GameScene::captureMouse() {
  if (EngineConstants::Input::ENABLE_MOUSE_CAPTURE) {
    m_mouseCapture = true;
    m_window.setMouseCursorVisible(false);
    sf::Mouse::setPosition(m_windowCenter, m_window);
    LOG_INFO("GameScene: Mouse captured for FPS-style controls");
  }
}

void GameScene::releaseMouse() {
  m_mouseCapture = false;
  m_window.setMouseCursorVisible(true);
  LOG_INFO("GameScene: Mouse released");
}

void GameScene::toggleGrid() {
  m_gridVisible = !m_gridVisible;
  if (m_gridVisible && !m_gridCreated) {
    createGrid();
  }
  LOG_INFO_STREAM("GameScene: Grid visibility toggled to " << (m_gridVisible ? "ON" : "OFF"));
}

void GameScene::createGrid() {
  if (m_gridCreated) return;
  
  LOG_INFO("GameScene: Creating 3D debug grid");
  
  float halfSize = EngineConstants::UI::GRID_3D_SIZE / 2.0f;
  float spacing = EngineConstants::UI::GRID_3D_SPACING;
  float majorSpacing = EngineConstants::UI::GRID_3D_MAJOR_SPACING;
  
  // Create grid lines in X-Z plane (Y = 0)
  for (float x = -halfSize; x <= halfSize; x += spacing) {
    bool isMajor = (fmod(abs(x), majorSpacing) < 0.001f);
    glm::vec3 color = isMajor ? glm::vec3(0.8f, 0.8f, 0.8f) : glm::vec3(0.4f, 0.4f, 0.4f);
    
    // Vertical lines (parallel to Z axis)
    auto lineEntity = m_entityManager.addEntity(EntityTag::TRIANGLE); // Reuse triangle tag for now
    lineEntity->add<CTransform3D>(glm::vec3(0.0f), glm::vec3(0.0f), glm::vec3(1.0f));
    lineEntity->add<CGridLine>(glm::vec3(x, 0, -halfSize), glm::vec3(x, 0, halfSize), color, 
                               EngineConstants::UI::GRID_3D_LINE_WIDTH, isMajor);
  }
  
  for (float z = -halfSize; z <= halfSize; z += spacing) {
    bool isMajor = (fmod(abs(z), majorSpacing) < 0.001f);
    glm::vec3 color = isMajor ? glm::vec3(0.8f, 0.8f, 0.8f) : glm::vec3(0.4f, 0.4f, 0.4f);
    
    // Horizontal lines (parallel to X axis)
    auto lineEntity = m_entityManager.addEntity(EntityTag::TRIANGLE); // Reuse triangle tag for now
    lineEntity->add<CTransform3D>(glm::vec3(0.0f), glm::vec3(0.0f), glm::vec3(1.0f));
    lineEntity->add<CGridLine>(glm::vec3(-halfSize, 0, z), glm::vec3(halfSize, 0, z), color,
                               EngineConstants::UI::GRID_3D_LINE_WIDTH, isMajor);
  }
  
  // Add coordinate axes (X=red, Y=green, Z=blue)
  auto xAxis = m_entityManager.addEntity(EntityTag::TRIANGLE);
  xAxis->add<CTransform3D>(glm::vec3(0.0f), glm::vec3(0.0f), glm::vec3(1.0f));
  xAxis->add<CGridLine>(glm::vec3(-halfSize, 0, 0), glm::vec3(halfSize, 0, 0), 
                        glm::vec3(1.0f, 0.0f, 0.0f), 0.05f, true); // Red X-axis
  
  auto zAxis = m_entityManager.addEntity(EntityTag::TRIANGLE);
  zAxis->add<CTransform3D>(glm::vec3(0.0f), glm::vec3(0.0f), glm::vec3(1.0f));
  zAxis->add<CGridLine>(glm::vec3(0, 0, -halfSize), glm::vec3(0, 0, halfSize), 
                        glm::vec3(0.0f, 0.0f, 1.0f), 0.05f, true); // Blue Z-axis
  
  auto yAxis = m_entityManager.addEntity(EntityTag::TRIANGLE);
  yAxis->add<CTransform3D>(glm::vec3(0.0f), glm::vec3(0.0f), glm::vec3(1.0f));
  yAxis->add<CGridLine>(glm::vec3(0, -halfSize/2, 0), glm::vec3(0, halfSize/2, 0), 
                        glm::vec3(0.0f, 1.0f, 0.0f), 0.05f, true); // Green Y-axis
  
  m_gridCreated = true;
  LOG_INFO_STREAM("GameScene: Created grid with " << (int)(2 * (halfSize * 2 / spacing + 1)) << " lines");
}

void GameScene::destroyGrid() {
  if (!m_gridCreated) return;
  
  LOG_INFO("GameScene: Destroying grid (not implemented - grid entities persist)");
  // TODO: Implement grid entity removal by tracking grid entities
  m_gridCreated = false;
}
