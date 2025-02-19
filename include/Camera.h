#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

class Camera {
public:
  glm::vec3 position;
  glm::vec3 front;
  glm::vec3 up;
  float yaw;
  float pitch;
  float fov;

  Camera(glm::vec3 startPosition);

  glm::mat4 getViewMatrix() const;
  glm::mat4 getProjectionMatrix(float aspectRatio) const;

  void processKeyboard(char direction,
                       float deltaTime); // TODO: replace this with Scene method

  void processMouse(float xOffset, float yOffset);
};
