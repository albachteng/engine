#include "../include/MapScene.h"
#include <memory>

void MapScene::onUnload() {};
void MapScene::update(float deltaTime) {};
void MapScene::render() {};

MapScene::MapScene(const std::shared_ptr<Entity> &player) : m_player(player){};

MapScene::MapScene(
    const std::shared_ptr<ActionController<MapActions>> inputController,
    const std::shared_ptr<Entity> &player)
    : m_player(player), m_actionController(inputController){};

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
