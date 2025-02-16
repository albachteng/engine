#pragma once
#include "BaseScene.hpp"
#include "Entity.hpp"
#include <cstddef>
#include <memory>

enum class SceneActions { UP, DOWN, LEFT, RIGHT, PAUSE };
enum class ActionType { PRESSED, RELEASED };

class GameScene : public BaseScene<SceneActions, ActionType> {
private:
  std::shared_ptr<Entity> m_player;
  bool m_paused = false;

public:
  // to avoid breaking polymorphism, dangerous
  // not all scenes will require a player pointer
  GameScene(const std::shared_ptr<Entity> &player = nullptr);
  void init() override;
  void togglePaused();
  bool isPaused();

protected:
  void handleAction(const SceneActions &action,
                    const ActionType &type) override;
};
