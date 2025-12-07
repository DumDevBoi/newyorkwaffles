// SFMLtest.cpp : Defines the entry point for the application.
//

#include "include.h"

int main() 
{
    sf::RenderWindow window(sf::VideoMode({ 1920, 1080 }), "SFML Window");
    window.setFramerateLimit(60);
    sf::View camera(sf::FloatRect({ 0.f, 0.f }, { 1920.f, 1080.f }));
    window.setView(camera);

    float cameraSpeed = 500.f;
    float zoomLevel = 1.f;
    const float zoomSpeed = 0.1f;
    const float minZoom = 0.1f;
    const float maxZoom = 5.f;

    sf::Texture waffleTexture;
    if (!waffleTexture.loadFromFile("waffle.png")) {
        return -1; 
    }
    sf::Sprite waffleSprite(waffleTexture);
    waffleSprite.setScale(sf::Vector2(0.025f, 0.025f));

    sf::FloatRect bounds = waffleSprite.getLocalBounds();
    waffleSprite.setOrigin(bounds.size / 2.f);


    sf::Vector2f wafflePos(400.f, 300.f);
    sf::Vector2f targetPos = wafflePos;

    const float speed = 200.f;

    const float gridSize = 100.f;


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

            if (const auto* wheel = event->getIf<sf::Event::MouseWheelScrolled>()) {
                if (wheel->wheel == sf::Mouse::Wheel::Vertical) {
                    if (wheel->delta > 0 && zoomLevel > minZoom) {
                        zoomLevel -= zoomSpeed;
                        zoomLevel = std::max(zoomLevel, minZoom);
                    }
                    else if (wheel->delta < 0 && zoomLevel < maxZoom) {
                        zoomLevel += zoomSpeed;
                        zoomLevel = std::min(zoomLevel, maxZoom);
                    }
                    camera.setSize(sf::Vector2f(1920.f * zoomLevel, 1080.f * zoomLevel));
                }
            }
        }

        sf::Vector2f cameraMove(0.f, 0.f);
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::W))
            cameraMove.y -= cameraSpeed * deltaTime * zoomLevel;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::S))
            cameraMove.y += cameraSpeed * deltaTime * zoomLevel;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::A))
            cameraMove.x -= cameraSpeed * deltaTime * zoomLevel;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::D))
            cameraMove.x += cameraSpeed * deltaTime * zoomLevel;

        camera.move(cameraMove);
        window.setView(camera);

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

        sf::Vector2f cameraCenter = camera.getCenter();
        sf::Vector2f cameraSize = camera.getSize();

        float left = cameraCenter.x - cameraSize.x / 2.f;
        float right = cameraCenter.x + cameraSize.x / 2.f;
        float top = cameraCenter.y - cameraSize.y / 2.f;
        float bottom = cameraCenter.y + cameraSize.y / 2.f;

        sf::RectangleShape gridLine;
        gridLine.setFillColor(sf::Color::Black); 

        float startX = std::floor(left / gridSize) * gridSize;
        for (float x = startX; x <= right; x += gridSize) {
            gridLine.setSize(sf::Vector2f(2.f * zoomLevel, cameraSize.y));
            gridLine.setPosition(sf::Vector2(x, top));
            window.draw(gridLine);
        }
        float startY = std::floor(top / gridSize) * gridSize;
        for (float y = startY; y <= bottom; y += gridSize) {
            gridLine.setSize(sf::Vector2f(cameraSize.x, 2.f * zoomLevel));
            gridLine.setPosition(sf::Vector2(left, y));
            window.draw(gridLine);
        }

        sf::Vertex line[] = {
        sf::Vertex(wafflePos, sf::Color::Blue),
        sf::Vertex(targetPos, sf::Color::Blue)
            };
        window.draw(line, 200, sf::PrimitiveType::Lines);

        window.draw(waffleSprite);
        
        window.display();
    }

    return 0;
}
