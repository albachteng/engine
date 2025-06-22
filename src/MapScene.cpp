#include "../include/MapScene.h"
#include "../include/Constants.hpp"
#include "../include/Logger.hpp"
#include <cstdlib>
#include <memory>
#include <cmath>
#include <algorithm>
#include <limits>

void MapScene::onUnload() {};

void MapScene::update(float deltaTime) { m_entityManager.update(); };

void MapScene::sRender() { m_renderer->render(m_entityManager.getEntities()); };

MapScene::MapScene(sf::RenderWindow &window) {
  m_player = spawnPlayer();
  m_window_size = (Vec2f)window.getSize(); // TODO: handle resize
  m_renderer = std::make_unique<SFMLRenderer>(window);
  m_actionController = std::make_shared<ActionController<MapActions>>();
  generateSampleMap();
};

std::shared_ptr<Entity> MapScene::spawnPlayer() {
  auto e = m_entityManager.addEntity(EntityTag::PLAYER);
  e->add<CInput>();
  return e;
};

void MapScene::onLoad() {
  LOG_INFO("MapScene: Initializing scene");
  LOG_DEBUG("MapScene: Registering input mappings");
  m_inputMap[InputEvent{InputType::Keyboard, sf::Keyboard::W}] = MapActions::UP;
  m_inputMap[InputEvent{InputType::Keyboard, sf::Keyboard::A}] =
      MapActions::LEFT;
  m_inputMap[InputEvent{InputType::Keyboard, sf::Keyboard::S}] =
      MapActions::DOWN;
  m_inputMap[InputEvent{InputType::Keyboard, sf::Keyboard::D}] =
      MapActions::RIGHT;
  m_inputMap[InputEvent{InputType::Keyboard, sf::Keyboard::Enter}] =
      MapActions::SELECT;
  m_inputMap[InputEvent{InputType::Keyboard, sf::Keyboard::Up}] =
      MapActions::UP;
  m_inputMap[InputEvent{InputType::Keyboard, sf::Keyboard::Left}] =
      MapActions::LEFT;
  m_inputMap[InputEvent{InputType::Keyboard, sf::Keyboard::Down}] =
      MapActions::DOWN;
  m_inputMap[InputEvent{InputType::Keyboard, sf::Keyboard::Right}] =
      MapActions::RIGHT;

  // register listeners
  LOG_DEBUG("MapScene: Registering input listeners");
  m_actionController->registerListener(MapActions::UP, [this](float deltaTime) {
    navigateInDirection(Direction::UP);
  });
  m_actionController->registerListener(
      MapActions::LEFT, [this](float deltaTime) { navigateInDirection(Direction::LEFT); });
  m_actionController->registerListener(
      MapActions::DOWN, [this](float deltaTime) { navigateInDirection(Direction::DOWN); });
  m_actionController->registerListener(
      MapActions::RIGHT, [this](float deltaTime) { navigateInDirection(Direction::RIGHT); });
  m_actionController->registerAxisListener(MapActions::CURSOR_MOVE,
                                           [this](float x, float y) {
                                             /* watch mouse movement */
                                           });
  LOG_INFO("MapScene: Scene loading completed");
};

void MapScene::moveCursor(int dx, int dy) {
  LOG_DEBUG_STREAM("MapScene: Previous selected node: " << m_selectedNode.x << ", "
            << m_selectedNode.y);
  int newX = m_selectedNode.x + dx;
  int newY = m_selectedNode.y + dy;

  if (newX >= 0 && newX < m_cols && newY >= 0 && newY < m_rows) {
    m_selectedNode = {newX, newY};
    LOG_DEBUG_STREAM("MapScene: New selected node: " << newX << ", " << newY);
  };
};

Vec2i MapScene::getCursor() { return m_selectedNode; };

void MapScene::processInput(const InputEvent &input, float deltaTime) {
  LOG_DEBUG("MapScene: Processing input");
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

void MapScene::togglePaused() { m_paused = !m_paused; };

bool MapScene::isPaused() { return m_paused; };

void MapScene::spawnMapNodes() {
  LOG_DEBUG("MapScene: Spawning map nodes");
  int cols = EngineConstants::UI::MAP_GRID_COLS;
  int rows = EngineConstants::UI::MAP_GRID_ROWS;
  float node_height = m_window_size.y / cols;
  float node_width = m_window_size.x / rows;
  for (int i = 0; i < cols; i++) {
    for (int j = 0; j < rows; j++) {

      LOG_DEBUG_STREAM("MapScene: Creating node at position: " << i << ", " << j);
      auto e = m_entityManager.addEntity(EntityTag::MAP_NODE);
      bool isSelected = i == 0 && j == 0;
      e->add<CShape>(node_height * EngineConstants::UI::MAP_NODE_SIZE_FACTOR, 
                     EngineConstants::UI::MAP_NODE_SHAPE_POINTS,
                     isSelected ? sf::Color::Black : sf::Color::White,
                     isSelected ? sf::Color::White : sf::Color::Cyan, 
                     EngineConstants::UI::MAP_NODE_OUTLINE_THICKNESS);
      float x = i * node_width + node_width * EngineConstants::UI::GRID_CENTER_OFFSET;
      float y = j * node_height + node_height * EngineConstants::UI::GRID_CENTER_OFFSET;
      e->add<CTransform>(Vec2f{x, y}, Vec2f{0.0f, 0.0f}, EngineConstants::UI::MAP_NODE_ROTATION_ANGLE);
      e->add<CSelection>(Vec2i{i, j});
    }
  }
};

void MapScene::sMovement(float deltaTime) {
  // Update node visuals including pulsing animation
  updateNodeVisuals(deltaTime);
};

void MapScene::sInput(sf::Event &event, float deltaTime) {
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
    if (abs(event.mouseMove.x - lastX) < EngineConstants::Input::MOUSE_MOVEMENT_THRESHOLD &&
        abs(event.mouseMove.y - lastY) < EngineConstants::Input::MOUSE_MOVEMENT_THRESHOLD)
      return; // Skip minor movements
    float xOffset = event.mouseMove.x - lastX;
    float yOffset = lastY - event.mouseMove.y; // inverted Y
    lastX = event.mouseMove.x;
    lastY = event.mouseMove.y;
    LOG_DEBUG_STREAM("MapScene: Frame delta time: " << deltaTime);
    processInput(InputEvent{InputType::MouseMove,
                            std::pair<float, float>{xOffset, yOffset}},
                 deltaTime);
    break;
  }
  default:
    break;
  }
};

// Enhanced navigation system implementation
void MapScene::navigateInDirection(Direction dir) {
  int nextNodeId = findClosestNodeInDirection(m_currentNodeId, dir);
  if (nextNodeId != -1 && nextNodeId != m_currentNodeId) {
    // Update selection
    for (auto &e : m_entityManager.getEntities(EntityTag::MAP_NODE)) {
      if (e->has<CMapNode>()) {
        auto &mapNode = e->get<CMapNode>();
        mapNode.isSelected = (mapNode.nodeId == nextNodeId);
        if (mapNode.isSelected) {
          mapNode.pulseTimer = 0.0f; // Reset pulse animation
        }
      }
    }
    m_currentNodeId = nextNodeId;
    LOG_DEBUG_STREAM("MapScene: Navigated to node " << nextNodeId << " in direction " << static_cast<int>(dir));
  } else {
    LOG_DEBUG_STREAM("MapScene: No valid node found in direction " << static_cast<int>(dir));
  }
}

int MapScene::findClosestNodeInDirection(int currentNodeId, Direction dir) {
  // Find current node position
  Vec2f currentPos;
  bool found = false;
  for (auto &e : m_entityManager.getEntities(EntityTag::MAP_NODE)) {
    if (e->has<CMapNode>()) {
      auto &mapNode = e->get<CMapNode>();
      if (mapNode.nodeId == currentNodeId) {
        currentPos = mapNode.position;
        found = true;
        break;
      }
    }
  }
  
  if (!found) return -1;
  
  // Direction vectors
  Vec2f dirVector;
  switch (dir) {
    case Direction::UP:    dirVector = Vec2f(0.0f, -1.0f); break;
    case Direction::DOWN:  dirVector = Vec2f(0.0f, 1.0f); break;
    case Direction::LEFT:  dirVector = Vec2f(-1.0f, 0.0f); break;
    case Direction::RIGHT: dirVector = Vec2f(1.0f, 0.0f); break;
  }
  
  int bestNodeId = -1;
  float bestScore = -1.0f; // Lower scores are better (closer and more aligned)
  
  for (auto &e : m_entityManager.getEntities(EntityTag::MAP_NODE)) {
    if (e->has<CMapNode>()) {
      auto &mapNode = e->get<CMapNode>();
      if (mapNode.nodeId == currentNodeId || !mapNode.isNavigable) continue;
      
      float score = calculateDirectionalScore(currentPos, mapNode.position, dir);
      if (score >= 0 && (bestScore < 0 || score < bestScore)) {
        bestScore = score;
        bestNodeId = mapNode.nodeId;
      }
    }
  }
  
  return bestNodeId;
}

float MapScene::calculateDirectionalScore(const Vec2f& from, const Vec2f& to, Direction dir) {
  Vec2f delta = to - from;
  float distance = sqrt(delta.x * delta.x + delta.y * delta.y);
  
  if (distance < 0.001f) return -1.0f; // Same position
  
  // Normalize delta
  Vec2f normalizedDelta = Vec2f(delta.x / distance, delta.y / distance);
  
  // Direction vectors
  Vec2f dirVector;
  switch (dir) {
    case Direction::UP:    dirVector = Vec2f(0.0f, -1.0f); break;
    case Direction::DOWN:  dirVector = Vec2f(0.0f, 1.0f); break;
    case Direction::LEFT:  dirVector = Vec2f(-1.0f, 0.0f); break;
    case Direction::RIGHT: dirVector = Vec2f(1.0f, 0.0f); break;
  }
  
  // Calculate dot product for directional alignment
  float dotProduct = normalizedDelta.x * dirVector.x + normalizedDelta.y * dirVector.y;
  
  // Require minimum directional alignment
  if (dotProduct < EngineConstants::UI::MAP_DIRECTIONAL_TOLERANCE) {
    return -1.0f; // Not in the right direction
  }
  
  // Score combines distance and directional alignment (lower is better)
  // More aligned nodes (higher dot product) get lower scores
  return distance * (2.0f - dotProduct);
}

void MapScene::updateNodeVisuals(float deltaTime) {
  for (auto &e : m_entityManager.getEntities(EntityTag::MAP_NODE)) {
    if (e->has<CMapNode>() && e->has<CShape>()) {
      auto &mapNode = e->get<CMapNode>();
      auto &shape = e->get<CShape>();
      
      if (mapNode.isSelected) {
        // Update pulse animation
        mapNode.pulseTimer += deltaTime * EngineConstants::UI::MAP_PULSE_SPEED;
        
        // Calculate pulsing alpha
        float pulseValue = (sin(mapNode.pulseTimer) + 1.0f) * 0.5f; // 0.0 to 1.0
        int alpha = static_cast<int>(EngineConstants::UI::MAP_PULSE_MIN_ALPHA + 
                   pulseValue * (EngineConstants::UI::MAP_PULSE_MAX_ALPHA - EngineConstants::UI::MAP_PULSE_MIN_ALPHA));
        
        // Apply selected colors with pulsing effect
        sf::Color pulseColor = mapNode.pulseColor;
        pulseColor.a = alpha;
        shape.circle.setFillColor(pulseColor);
        shape.circle.setOutlineColor(mapNode.selectedColor);
        shape.circle.setOutlineThickness(EngineConstants::UI::MAP_SELECTION_OUTLINE);
      } else {
        // Standard appearance for non-selected nodes
        shape.circle.setFillColor(mapNode.baseColor);
        shape.circle.setOutlineColor(sf::Color::Cyan);
        shape.circle.setOutlineThickness(EngineConstants::UI::MAP_NODE_OUTLINE_THICKNESS);
      }
    }
  }
}

void MapScene::generateSampleMap() {
  LOG_DEBUG("MapScene: Generating sample map with arbitrary node positions");
  m_nodeIds.clear();
  
  // Create a sample map with various arrangements to test navigation
  std::vector<std::pair<Vec2f, NodeShape>> nodeData = {
    // Center cluster
    {Vec2f(400, 300), NodeShape::CIRCLE},    // 0 - Center
    {Vec2f(300, 300), NodeShape::SQUARE},    // 1 - Left
    {Vec2f(500, 300), NodeShape::DIAMOND},   // 2 - Right
    {Vec2f(400, 200), NodeShape::TRIANGLE},  // 3 - Up
    {Vec2f(400, 400), NodeShape::HEXAGON},   // 4 - Down
    
    // Extended positions
    {Vec2f(200, 300), NodeShape::CIRCLE},    // 5 - Far left
    {Vec2f(600, 300), NodeShape::SQUARE},    // 6 - Far right
    {Vec2f(400, 100), NodeShape::DIAMOND},   // 7 - Far up
    {Vec2f(400, 500), NodeShape::TRIANGLE},  // 8 - Far down
    
    // Diagonal positions
    {Vec2f(350, 250), NodeShape::HEXAGON},   // 9 - Up-left
    {Vec2f(450, 250), NodeShape::CIRCLE},    // 10 - Up-right
    {Vec2f(350, 350), NodeShape::SQUARE},    // 11 - Down-left
    {Vec2f(450, 350), NodeShape::DIAMOND},   // 12 - Down-right
  };
  
  for (size_t i = 0; i < nodeData.size(); ++i) {
    auto node = createMapNode(static_cast<int>(i), nodeData[i].first, nodeData[i].second);
    m_nodeIds.push_back(static_cast<int>(i));
  }
  
  // Process deferred entity additions
  m_entityManager.update();
  
  // Set initial selection
  m_currentNodeId = 0;
  for (auto &e : m_entityManager.getEntities(EntityTag::MAP_NODE)) {
    if (e->has<CMapNode>()) {
      auto &mapNode = e->get<CMapNode>();
      mapNode.isSelected = (mapNode.nodeId == 0);
    }
  }
  
  LOG_INFO_STREAM("MapScene: Generated sample map with " << nodeData.size() << " nodes");
}

std::shared_ptr<Entity> MapScene::createMapNode(int nodeId, const Vec2f& position, NodeShape shape) {
  auto e = m_entityManager.addEntity(EntityTag::MAP_NODE);
  
  // Create the visual shape
  int shapePoints;
  switch (shape) {
    case NodeShape::CIRCLE:   shapePoints = 30; break;
    case NodeShape::SQUARE:   shapePoints = 4; break;
    case NodeShape::DIAMOND:  shapePoints = 4; break;
    case NodeShape::TRIANGLE: shapePoints = 3; break;
    case NodeShape::HEXAGON:  shapePoints = 6; break;
    default: shapePoints = 30; break;
  }
  
  e->add<CShape>(EngineConstants::UI::MAP_NODE_DEFAULT_SIZE, shapePoints,
                 sf::Color::White, sf::Color::Cyan, EngineConstants::UI::MAP_NODE_OUTLINE_THICKNESS);
  
  // Set position
  e->add<CTransform>(position, Vec2f{0.0f, 0.0f}, 
                     shape == NodeShape::DIAMOND ? EngineConstants::UI::MAP_NODE_ROTATION_ANGLE : 0.0f);
  
  // Add map node component with enhanced data
  e->add<CMapNode>(nodeId, position, shape);
  
  // Keep legacy selection component for compatibility
  e->add<CSelection>(Vec2i{nodeId % m_cols, nodeId / m_cols});
  
  return e;
}
