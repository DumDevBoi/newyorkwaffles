// SFMLtest.cpp : Defines the entry point for the application.
//

#include "include.h"

int main()
{
    sf::RenderWindow window(sf::VideoMode({ 1920, 1080 }), "SFML Window");

    while (window.isOpen())
    {
        while (auto event = window.pollEvent())
        {
            if (event->is<sf::Event::Closed>())
                window.close();
        }

        window.clear(sf::Color::Green);
        window.display();
    }

    return 0;
}
