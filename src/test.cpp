#include "../include/Game.h"
#include <SFML/Graphics.hpp>
#include <iostream>

int main() {

  std::cout << "different" << std::endl;
  Game g = Game{"path"};
  g.run();
  return 0;
}
