#include "./EntityManager.cpp"
#include <SFML/Graphics.hpp>
#include <iostream>

int main() {

  EntityManager em;
  em.addEntity("bullet");
  em.addEntity("enemy");
  em.addEntity("player");
  //   std::cout << em.getEntities("bullet").at(0) << std::endl;
  // std::cout << em.getEntities("enemy").at(0) << std::endl;
  sf::RenderWindow window(sf::VideoMode(800, 600), "SFML Test");
  while (window.isOpen()) {
    sf::Event event;
    while (window.pollEvent(event)) {
      if (event.type == sf::Event::Closed)
        window.close();
    }
    window.clear(sf::Color::Black);
    window.display();
  }
  return 0;
}
