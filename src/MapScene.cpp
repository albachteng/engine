#include "../include/MapScene.h"
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
  std::cout << "Init fired in MapScene" << std::endl;
  std::cout << "registering input map" << std::endl;
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
  std::cout << "registering listeners" << std::endl;
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
                                             // std::cout << "firing axis
                                             // listener: " << x << ", " << y <<
                                             // std::endl;
                                             /* watch mouse movement */
                                           });
  std::cout << "finished onLoad()" << std::endl;
};

void MapScene::moveCursor(int dx, int dy) {
  std::cout << "previous selected node: " << m_selectedNode.x << ", "
            << m_selectedNode.y << std::endl;
  int newX = m_selectedNode.x + dx;
  int newY = m_selectedNode.y + dy;

  if (newX >= 0 && newX < m_cols && newY >= 0 && newY < m_rows) {
    m_selectedNode = {newX, newY};
    std::cout << "new selected node: " << newX << ", " << newY << std::endl;
  };
};

Vec2i MapScene::getCursor() { return m_selectedNode; };

void MapScene::processInput(const InputEvent &input, float deltaTime) {
  std::cout << "processInput" << std::endl;
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
  std::cout << "entering spawnMapNodes" << std::endl;
  int cols = 10;
  int rows = 10;
  float node_height = m_window_size.y / cols;
  float node_width = m_window_size.x / rows;
  for (int i = 0; i < cols; i++) {
    for (int j = 0; j < rows; j++) {

      std::cout << "spawn map nodes loop: " << i << ", " << j << std::endl;
      auto e = m_entityManager.addEntity(EntityTag::MAP_NODE);
      bool isSelected = i == 0 && j == 0;
      e->add<CShape>(node_height * .4f, 4,
                     isSelected ? sf::Color::Black : sf::Color::White,
                     isSelected ? sf::Color::White : sf::Color::Cyan, 3.0f);
      float x = i * node_width + node_width * 0.5f;
      float y = j * node_height + node_height * 0.5f;
      e->add<CTransform>(Vec2f{x, y}, Vec2f{0.0f, 0.0f}, 45.0f);
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
    std::cout << "movement: " << cursor.x << cursor.y << std::endl;
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
    if (abs(event.mouseMove.x - lastX) < 2 &&
        abs(event.mouseMove.y - lastY) < 2)
      return; // Skip minor movements
    float xOffset = event.mouseMove.x - lastX;
    float yOffset = lastY - event.mouseMove.y; // inverted Y
    lastX = event.mouseMove.x;
    lastY = event.mouseMove.y;
    std::cout << "delta: " << deltaTime << std::endl;
    processInput(InputEvent{InputType::MouseMove,
                            std::pair<float, float>{xOffset, yOffset}},
                 deltaTime);
    break;
  }
  default:
    break;
  }
};
