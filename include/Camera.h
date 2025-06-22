#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

enum class CameraMovement { FORWARD, BACK, LEFT, RIGHT };

class Camera {
private:
  glm::vec3 position;
  glm::vec3 front;
  glm::vec3 up;
  float yaw;
  float pitch;
  float fov;

public:
  Camera(glm::vec3 startPosition);

  glm::mat4 getViewMatrix() const;

  glm::mat4 getProjectionMatrix(float aspectRatio) const;

  void move(CameraMovement movement,
            float deltaTime); // TODO: replace this with Scene method

  void rotate(float xOffset, float yOffset);           // Applies built-in sensitivity scaling
  void rotateRaw(float xOffset, float yOffset);       // Uses pre-processed values without additional scaling
};
