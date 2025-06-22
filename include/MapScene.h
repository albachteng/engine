
#include "BaseScene.hpp"
#include "Component.h"
#include "Entity.hpp"
#include "EntityManager.h"
#include "InputController.hpp"
#include "SFMLRenderer.h"
#include <memory>
#include <unordered_map>
#include <vector>
#include <cmath>

enum class MapActions { UP, DOWN, LEFT, RIGHT, CURSOR_MOVE, SELECT };

enum class Direction {
  UP,
  DOWN,
  LEFT,
  RIGHT
};

class MapScene : public BaseScene {
private:
  std::unique_ptr<SFMLRenderer> m_renderer;
  Vec2f m_window_size;

  std::unordered_map<InputEvent, MapActions> m_inputMap;
  std::shared_ptr<Entity> m_player;
  std::shared_ptr<ActionController<MapActions>> m_actionController;
  bool m_paused = false;
  int m_cols = 10;
  int m_rows = 10;
  
  // Enhanced navigation system
  std::vector<int> m_nodeIds;            // List of all node IDs for easy access
  int findClosestNodeInDirection(int currentNodeId, Direction dir);
  
  // Legacy support (can be removed later)
  void moveCursor(int dx, int dy);
  Vec2i m_selectedNode = {0, 0};

  std::shared_ptr<Entity> spawnPlayer();
  void spawnMapNodes();
  void generateSampleMap();              // Creates a sample map with various node arrangements
  std::shared_ptr<Entity> createMapNode(int nodeId, const Vec2f& position, NodeShape shape = NodeShape::CIRCLE);

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
  
  // Public for testing
  EntityManager m_entityManager;
  int m_currentNodeId = 0;
  void navigateInDirection(Direction dir);
  float calculateDirectionalScore(const Vec2f& from, const Vec2f& to, Direction dir);
  void updateNodeVisuals(float deltaTime);
};
