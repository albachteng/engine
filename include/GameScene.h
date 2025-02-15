#pragma once
#include "BaseScene.hpp"
#include "Entity.hpp"
#include <cstddef>
#include <memory>

enum class SceneActions { UP, DOWN, LEFT, RIGHT, PAUSE };

class GameScene : public BaseScene<SceneActions> {
private:
  std::shared_ptr<Entity> m_player;

public:
  // to avoid breaking polymorphism, dangerous
  // not all scenes will require a player pointer
  GameScene(std::shared_ptr<Entity> player = nullptr);
  void init() override;

protected:
  void handleAction(const SceneActions &action) override;
};
