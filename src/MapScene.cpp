#include "../include/MapScene.h"
#include "../include/Constants.hpp"
#include "../include/Logger.hpp"
#include <cstdlib>
#include <memory>

void MapScene::onUnload() {};

void MapScene::update(float deltaTime) { m_entityManager.update(); };

void MapScene::sRender() { m_renderer->render(m_entityManager.getEntities()); };

MapScene::MapScene(sf::RenderWindow &window) {
  m_player = spawnPlayer();
  m_window_size = (Vec2f)window.getSize(); // TODO: handle resize
  m_renderer = std::make_unique<SFMLRenderer>(window);
  m_actionController = std::make_shared<ActionController<MapActions>>();
  spawnMapNodes();
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
    /* map movement */
  });
  m_actionController->registerListener(
      MapActions::LEFT, [this](float deltaTime) { moveCursor(-1, 0); });
  m_actionController->registerListener(
      MapActions::DOWN, [this](float deltaTime) { moveCursor(0, 1); });
  m_actionController->registerListener(
      MapActions::RIGHT, [this](float deltaTime) { moveCursor(1, 0); });
  m_actionController->registerListener(
      MapActions::UP, [this](float deltaTime) { moveCursor(0, -1); });
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
  float height = m_window_size.y;
  float width = m_window_size.x;
  // MapNode selection updates
  for (auto &e : m_entityManager.getEntities(EntityTag::MAP_NODE)) {
    auto cursor = getCursor();
    LOG_DEBUG_STREAM("MapScene: Cursor movement: " << cursor.x << ", " << cursor.y);
    auto selection = e->get<CSelection>();
    auto &shape = e->get<CShape>();

    if (cursor == selection.grid_position) {
      shape.circle.setFillColor(sf::Color::Black);
      shape.circle.setOutlineColor(sf::Color::White);
    } else {
      shape.circle.setFillColor(sf::Color::White);
      shape.circle.setOutlineColor(sf::Color::Cyan);
    }
  };
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
