#pragma once
#include "../include/Camera.h"
#include "InputController.hpp"
#include <memory>

class CameraController {
private:
  std::shared_ptr<Camera> m_camera;
  std::shared_ptr<IInputController> m_inputController;

public:
  CameraController(std::shared_ptr<Camera> camera,
                   std::shared_ptr<IInputController> input);
  void handleEvent(const InputEvent &event, float x = 0.0f, float y = 0.0f,
                   float deltaTime = 0.0f);
  void registerControls();
  std::shared_ptr<Camera> camera();
  std::shared_ptr<IInputController> inputController();
};
