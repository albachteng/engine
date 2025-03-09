#include "../include/Game.h"
#include <SFML/Graphics.hpp>
#include <glad/glad.h>
#include <iostream>

int main() {
  Game g = Game{"path"};
  // if (!gladLoadGL()) {
  //   std::cerr << "failed to initialize GLAD" << std::endl;
  //   return -1;
  // }
  g.run();
  return 0;
}
