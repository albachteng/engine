
#include "BaseScene.hpp"
#include "Entity.hpp"
#include "InputController.hpp"
#include <memory>

enum class MapActions { UP, DOWN, LEFT, RIGHT, SELECT };

class MapScene : BaseScene<MapActions> {
private:
  std::shared_ptr<Entity> m_player;
  std::shared_ptr<ActionController<MapActions>> m_actionController;

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
};
