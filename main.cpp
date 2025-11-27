// SFMLtest.cpp : Defines the entry point for the application.
//

#include "include.h"
#include "GameObject.h"
#include "Component.h"

int main() 
{
    sf::RenderWindow window(sf::VideoMode({ 1920, 1080 }), "SFML Window");
    window.setFramerateLimit(60);

    //Test 
    GameObject unit;
    auto move = unit.addComponent<MovementComponent>(670.f);
    unit.addComponent<RenderComponent>("waffle.png"); 
    unit.x = 500;
    unit.y = 500;
    //Test 

    sf::Clock clock;
    while (window.isOpen())
    {
		float deltaTime = clock.restart().asSeconds();
        while (auto event = window.pollEvent())
        {
            if (event->is<sf::Event::Closed>())
                window.close();
            //test
            if (auto m = event->getIf<sf::Event::MouseButtonPressed>()) {
                if (m->button == sf::Mouse::Button::Left) {
                    move->target = (sf::Vector2f)sf::Mouse::getPosition(window);
                }
            }
            //test
        }

        window.clear(sf::Color::Green);
        unit.draw(window);
        window.display();
    }

    return 0;
}
