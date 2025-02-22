#include "../include/Camera.h"
#include <memory>

class CameraController {
private:
  std::shared_ptr<Camera> m_camera;

public:
  CameraController(std::shared_ptr<Camera> camera);
  void handleKeyboard(CameraMovement movement, float deltaTime);
  void handleMouse(float xOffset, float yOffset);
  std::shared_ptr<Camera> camera();
};
