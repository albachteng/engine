#include "../include/Game.h"
#include "../include/Logger.hpp"
#include <SFML/Graphics.hpp>
#include <glad/glad.h>

int main() {
  // Initialize logging system
  Logger::initialize(LogLevel::INFO, LogOutput::BOTH, "engine.log");
  LOG_INFO("Engine: Starting application");
  
  try {
    Game g = Game{"path"};
    // if (!gladLoadGL()) {
    //   LOG_ERROR("Failed to initialize GLAD");
    //   return -1;
    // }
    g.run();
    LOG_INFO("Engine: Application shutting down normally");
  } catch (const std::exception& e) {
    LOG_FATAL_STREAM("Engine: Unhandled exception: " << e.what());
    return -1;
  }
  
  Logger::shutdown();
  return 0;
}
