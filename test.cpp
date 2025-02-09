#include "./Game.cpp"
#include <SFML/Graphics.hpp>

int main() {

  EntityManager em;
  Game g = Game{"path"};
  g.run();
  return 0;
}
