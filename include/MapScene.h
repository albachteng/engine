
#include "BaseScene.hpp"
#include "Component.h"
#include "Entity.hpp"
#include "EntityManager.h"
#include "InputController.hpp"
#include "SFMLRenderer.h"
#include <memory>
#include <unordered_map>

enum class MapActions { UP, DOWN, LEFT, RIGHT, CURSOR_MOVE, SELECT };

class MapScene : public BaseScene {
private:
  EntityManager m_entityManager;
  std::unique_ptr<SFMLRenderer> m_renderer;
  Vec2f m_window_size;

  std::unordered_map<InputEvent, MapActions> m_inputMap;
  std::shared_ptr<Entity> m_player;
  std::shared_ptr<ActionController<MapActions>> m_actionController;
  bool m_paused = false;
  int m_cols = 10;
  int m_rows = 10;
  void moveCursor(int dx, int dy);
  Vec2i m_selectedNode = {0, 0};

  std::shared_ptr<Entity> spawnPlayer();
  void spawnMapNodes();

public:
  explicit MapScene(sf::RenderWindow &window);

  void sMovement(float deltaTime = 0.0f) override;
  void sInput(sf::Event &event, float deltaTime) override;
  void onLoad() override;
  void onUnload() override;
  void update(float deltaTime) override;
  void processInput(const InputEvent &event, float deltaTime = 0.0f) override;
  void sRender() override;

  void togglePaused();
  bool isPaused();
  std::shared_ptr<ActionController<MapActions>> actionController();

  Vec2i getCursor();
};
