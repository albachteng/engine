#include "../include/Game.h"
#include <SFML/Graphics.hpp>
#include <iostream>

int main() {
  Game g = Game{"path"};
  g.run();
  return 0;
}
