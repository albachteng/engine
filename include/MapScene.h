
#include "BaseScene.hpp"
#include "Entity.hpp"
#include "EntityManager.h"
#include "InputController.hpp"
#include <memory>
#include <unordered_map>

enum class MapActions { UP, DOWN, LEFT, RIGHT, CURSOR_MOVE, SELECT };

class MapScene : BaseScene {
private:
  EntityManager m_entityManager;
  std::unordered_map<InputEvent, MapActions> m_inputMap;
  std::shared_ptr<Entity> m_player;
  std::shared_ptr<ActionController<MapActions>> m_actionController;
  bool m_paused = false;
  int m_cols = 10;
  int m_rows = 10;
  void moveCursor(int dx, int dy);
  Vec2i m_selectedNode = {0, 0};

public:
  MapScene(const std::shared_ptr<Entity> &player = nullptr);
  MapScene(const std::shared_ptr<ActionController<MapActions>> actionController,
           const std::shared_ptr<Entity> &player = nullptr);

  void onLoad() override;
  void onUnload() override;
  void update(float deltaTime) override;
  void processInput(const InputEvent &event, float deltaTime = 0.0f) override;
  void render() override;

  void togglePaused();
  bool isPaused();
  std::shared_ptr<ActionController<MapActions>> actionController();

  Vec2i getCursor();
};
