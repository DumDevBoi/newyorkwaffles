// SFMLtest.cpp : Defines the entry point for the application.
//

#include "include.h"

struct Waffle {
    sf::Vector2f pos;
    sf::Vector2f targetPos;
    bool isSelected = false;

    Waffle(sf::Vector2f position) : pos(position), targetPos(position) {}
};

int main()
{
    sf::RenderWindow window(sf::VideoMode({ 1920, 1080 }), "SFML Window");
    window.setFramerateLimit(60);

    // Camera setup
    sf::View camera(sf::FloatRect({ 0.f, 0.f }, { 1920.f, 1080.f }));
    window.setView(camera);

    float cameraSpeed = 500.f;
    float zoomLevel = 1.f;
    const float zoomSpeed = 0.1f;
    const float minZoom = 0.1f;
    const float maxZoom = 5.f;

    // Waffle setup
    sf::Texture waffleTexture;
    if (!waffleTexture.loadFromFile("waffle.png")) {
        return -1;
    }

    // Create multiple waffles
    std::vector<Waffle> waffles;
    waffles.emplace_back(sf::Vector2f(0.f, 0.f));
    waffles.emplace_back(sf::Vector2f(200.f, 0.f));
    waffles.emplace_back(sf::Vector2f(-200.f, 0.f));
    waffles.emplace_back(sf::Vector2f(0.f, 200.f));
    waffles.emplace_back(sf::Vector2f(0.f, -200.f));

    const float speed = 1000.f;
    const float waffleRadius = 50.f; // Collision radius for selection

    // Selection state
    bool isDragging = false;
    sf::Vector2f dragStart;
    sf::RectangleShape selectionBox;
    selectionBox.setFillColor(sf::Color(100, 100, 255, 50));
    selectionBox.setOutlineColor(sf::Color::Blue);
    selectionBox.setOutlineThickness(2.f);

    // Grid setup for infinite plane
    const float gridSize = 100.f;

    sf::Clock clock;

    while (window.isOpen())
    {
        float deltaTime = clock.restart().asSeconds();

        while (auto event = window.pollEvent())
        {
            if (event->is<sf::Event::Closed>())
                window.close();

            // Left mouse button - selection
            if (const auto* mouseButton = event->getIf<sf::Event::MouseButtonPressed>()) {
                if (mouseButton->button == sf::Mouse::Button::Left) {
                    isDragging = true;
                    dragStart = window.mapPixelToCoords(mouseButton->position);
                }
            }

            if (const auto* mouseButton = event->getIf<sf::Event::MouseButtonReleased>()) {
                if (mouseButton->button == sf::Mouse::Button::Left && isDragging) {
                    isDragging = false;
                    sf::Vector2f dragEnd = window.mapPixelToCoords(mouseButton->position);

                    // Create selection rectangle
                    float minX = std::min(dragStart.x, dragEnd.x);
                    float maxX = std::max(dragStart.x, dragEnd.x);
                    float minY = std::min(dragStart.y, dragEnd.y);
                    float maxY = std::max(dragStart.y, dragEnd.y);

                    // Check if it was just a click (small drag area)
                    bool isClick = std::abs(dragEnd.x - dragStart.x) < 5.f &&
                        std::abs(dragEnd.y - dragStart.y) < 5.f;

                    // Clear previous selection if not holding shift
                    if (!sf::Keyboard::isKeyPressed(sf::Keyboard::Key::LShift)) {
                        for (auto& w : waffles) {
                            w.isSelected = false;
                        }
                    }

                    // Select waffles
                    for (auto& w : waffles) {
                        if (isClick) {
                            // Click selection - check radius
                            float dx = w.pos.x - dragStart.x;
                            float dy = w.pos.y - dragStart.y;
                            float dist = std::sqrt(dx * dx + dy * dy);
                            if (dist <= waffleRadius) {
                                w.isSelected = !w.isSelected;
                                break; // Only select one waffle on click
                            }
                        }
                        else {
                            // Box selection
                            if (w.pos.x >= minX && w.pos.x <= maxX &&
                                w.pos.y >= minY && w.pos.y <= maxY) {
                                w.isSelected = true;
                            }
                        }
                    }
                }
            }

            // Right mouse button - move selected waffles
            if (const auto* mouseButton = event->getIf<sf::Event::MouseButtonPressed>()) {
                if (mouseButton->button == sf::Mouse::Button::Right) {
                    sf::Vector2f clickPos = window.mapPixelToCoords(mouseButton->position);
                    for (auto& w : waffles) {
                        if (w.isSelected) {
                            w.targetPos = clickPos;
                        }
                    }
                }
            }

            // Zoom with mouse wheel
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

        // Camera movement with WASD
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

        // Update all waffles
        for (auto& w : waffles) {
            sf::Vector2f direction = w.targetPos - w.pos;
            float distance = std::sqrt(direction.x * direction.x + direction.y * direction.y);

            if (distance > 1.f) {
                direction /= distance;
                sf::Vector2f movement = direction * speed * deltaTime;

                if (std::sqrt(movement.x * movement.x + movement.y * movement.y) > distance) {
                    w.pos = w.targetPos;
                }
                else {
                    w.pos += movement;
                }
            }
        }

        // Clear with green
        window.clear(sf::Color::Green);

        // Draw infinite grid
        sf::Vector2f cameraCenter = camera.getCenter();
        sf::Vector2f cameraSize = camera.getSize();

        float left = cameraCenter.x - cameraSize.x / 2.f;
        float right = cameraCenter.x + cameraSize.x / 2.f;
        float top = cameraCenter.y - cameraSize.y / 2.f;
        float bottom = cameraCenter.y + cameraSize.y / 2.f;

        sf::RectangleShape gridLine;
        gridLine.setFillColor(sf::Color(0, 150, 0));

        // Vertical lines
        float startX = std::floor(left / gridSize) * gridSize;
        for (float x = startX; x <= right; x += gridSize) {
            gridLine.setSize(sf::Vector2f(2.f * zoomLevel, cameraSize.y));
            gridLine.setPosition(sf::Vector2(x, top));
            window.draw(gridLine);
        }

        // Horizontal lines
        float startY = std::floor(top / gridSize) * gridSize;
        for (float y = startY; y <= bottom; y += gridSize) {
            gridLine.setSize(sf::Vector2f(cameraSize.x, 2.f * zoomLevel));
            gridLine.setPosition(sf::Vector2(left, y));
            window.draw(gridLine);
        }

        // Draw selection box while dragging
        if (isDragging) {
            sf::Vector2f currentMouse = window.mapPixelToCoords(sf::Mouse::getPosition(window));
            sf::Vector2f boxPos(std::min(dragStart.x, currentMouse.x),
                std::min(dragStart.y, currentMouse.y));
            sf::Vector2f boxSize(std::abs(currentMouse.x - dragStart.x),
                std::abs(currentMouse.y - dragStart.y));
            selectionBox.setPosition(boxPos);
            selectionBox.setSize(boxSize);
            window.draw(selectionBox);
        }

        // Draw waffles and lines
        for (const auto& w : waffles) {
            // Draw blue line if selected and moving
            if (w.isSelected) {
                sf::Vertex line[] = {
                    sf::Vertex(w.pos, sf::Color::Blue),
                    sf::Vertex(w.targetPos, sf::Color::Blue)
                };
                window.draw(line, 2, sf::PrimitiveType::Lines);
            }

            // Draw waffle sprite
            sf::Sprite waffleSprite(waffleTexture);
            waffleSprite.setScale(sf::Vector2f(0.035f, 0.035f));
            sf::FloatRect bounds = waffleSprite.getLocalBounds();
            waffleSprite.setOrigin(bounds.size / 2.f);
            waffleSprite.setPosition(w.pos);
            window.draw(waffleSprite);

            // Draw selection circle if selected
            if (w.isSelected) {
                sf::CircleShape selectionCircle(waffleRadius);
                selectionCircle.setOrigin(sf::Vector2(waffleRadius, waffleRadius));
                selectionCircle.setPosition(w.pos);
                selectionCircle.setFillColor(sf::Color::Transparent);
                selectionCircle.setOutlineColor(sf::Color::Yellow);
                selectionCircle.setOutlineThickness(3.f * zoomLevel);
                window.draw(selectionCircle);
            }
        }

        window.display();
    }

    return 0;
}