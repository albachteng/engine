#pragma once

#include "BaseScene.hpp"
#include <functional>
#include <memory>
#include <unordered_map>

class SceneManager {
private:
  std::unordered_map<std::string, std::function<std::shared_ptr<BaseScene>()>>
      m_sceneFactories;
  std::shared_ptr<BaseScene> m_currentScene;

public:
  void registerScene(const std::string &name,
                     std::function<std::shared_ptr<BaseScene>()> factory) {
    m_sceneFactories[name] = factory;
  }

  void loadScene(const std::string &name) {
    if (m_sceneFactories.find(name) != m_sceneFactories.end()) {
      if (m_currentScene) {
        m_currentScene->onUnload();
      }
    }

    m_currentScene = m_sceneFactories[name]();
    m_currentScene->onLoad();
  }

  std::shared_ptr<BaseScene> getCurrentScene() { return m_currentScene; }
};
