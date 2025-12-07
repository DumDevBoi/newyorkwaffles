// SFMLtest.cpp : Defines the entry point for the application.
//

#include "include.h"

int main() 
{
    sf::RenderWindow window(sf::VideoMode({ 1920, 1080 }), "SFML Window");
    window.setFramerateLimit(60);

    sf::Texture waffleTexture;
    if (!waffleTexture.loadFromFile("waffle.png")) {
        return -1; 
    }
    sf::Sprite waffleSprite(waffleTexture);
    waffleSprite.setScale(sf::Vector2(0.1f, 0.1f));

    sf::FloatRect bounds = waffleSprite.getLocalBounds();
    waffleSprite.setOrigin(bounds.size / 2.f);


    sf::Vector2f wafflePos(400.f, 300.f);
    sf::Vector2f targetPos = wafflePos;

    const float speed = 200.f;


    sf::Clock clock;
    while (window.isOpen())
    {
		float deltaTime = clock.restart().asSeconds();
        while (auto event = window.pollEvent())
        {
            if (event->is<sf::Event::Closed>())
                window.close();

            if (const auto* mouseButton = event->getIf<sf::Event::MouseButtonPressed>()) {
                if (mouseButton->button == sf::Mouse::Button::Right) {
                    targetPos = sf::Vector2f(mouseButton->position);
                }
            }


        }

        sf::Vector2f direction = targetPos - wafflePos;
        float distance = std::sqrt(direction.x * direction.x + direction.y * direction.y);

        if (distance > 1.f) {
            direction /= distance; 
            sf::Vector2f movement = direction * speed * deltaTime;


            if (std::sqrt(movement.x * movement.x + movement.y * movement.y) > distance) {
                wafflePos = targetPos;
            }
            else {
                wafflePos += movement;
            }
        }

        waffleSprite.setPosition(wafflePos);

        window.clear(sf::Color::Green);

        window.draw(waffleSprite);
        
        window.display();
    }

    return 0;
}
