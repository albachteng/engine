
#include "BaseScene.hpp"
#include "Entity.hpp"
#include "InputController.hpp"
#include <memory>

enum class MapActions { UP, DOWN, LEFT, RIGHT, CURSOR_MOVE, SELECT };

class MapScene : BaseScene<MapActions> {
private:
  std::shared_ptr<Entity> m_player;
  std::shared_ptr<ActionController<MapActions>> m_actionController;
  bool m_paused = false;
  int m_cols = 10;
  int m_rows = 10;
  void moveCursor(int dx, int dy);
  Vec2i m_selectedNode = {0, 0};

public:
  std::unordered_map<InputEvent, MapActions> m_inputMap;
  MapScene(const std::shared_ptr<Entity> &player = nullptr);
  MapScene(const std::shared_ptr<ActionController<MapActions>> actionController,
           const std::shared_ptr<Entity> &player = nullptr);
  void init() override;
  void togglePaused();
  bool isPaused();
  std::shared_ptr<ActionController<MapActions>> actionController();

  void processInput(const InputEvent &event, float deltaTime = 0.0f) override;
  Vec2i getCursor();
};
