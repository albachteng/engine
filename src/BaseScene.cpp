#include "../include/BaseScene.h"

void Scene::registerAction(int inputKey, const std::string &name) {
  m_actionMap[inputKey] = name;
}
