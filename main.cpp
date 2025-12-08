#include "include.h"

const float speed = 1000.f;
const float selectionRadius = 50.f;
const float collisionRadius = 57.f;
const float gridSize = 100.f;

struct Waffle {
    sf::Vector2f pos;
    sf::Vector2f targetPos;
    bool isSelected = false;
    std::vector<sf::Vector2f> path; // world positions of path nodes (cell centers)
    size_t pathIndex = 0;           // next node to move toward

    Waffle(sf::Vector2f position) : pos(position), targetPos(position) {}
};

bool isWall(int gx, int gy) {
    // Don't spawn walls ard (0,0) 
    if (std::abs(gx) <= 2 && std::abs(gy) <= 2) return false;

    // bit-shift grid coord, create unique 'seed' for every cell
    long long n = (long long)gx * 374761393 + (long long)gy * 668265263;
    n = (n ^ (n >> 13)) * 1274126177;

    // Returns true if the noise value is below a certain %
    // This creates a 25% chance of a block appearing
    return ((n ^ (n >> 16)) & 100) < 25;
}

struct ANode {
    int gx, gy;
    float g = 0.f;
    float h = 0.f;
    float f() const { return g + h; }
    std::pair<int, int> parent = { INT_MIN, INT_MIN };
};

static inline float heuristic(int ax, int ay, int bx, int by) {
    // use Manhattan or Euclidean; Euclidean provides smoother cost
    float dx = float(ax - bx);
    float dy = float(ay - by);
    return std::sqrt(dx * dx + dy * dy);
}

static inline sf::Vector2f gridToWorldCenter(int gx, int gy) {
    // center of cell
    return sf::Vector2f(gx * gridSize + gridSize * 0.5f, gy * gridSize + gridSize * 0.5f);
}

static inline std::vector<std::pair<int, int>> getNeighbors(int gx, int gy) {
    // 8-connected (diagonals allowed) — adjust if you want 4-connected
    std::vector<std::pair<int, int>> n;
    for (int dx = -1; dx <= 1; ++dx) {
        for (int dy = -1; dy <= 1; ++dy) {
            if (dx == 0 && dy == 0) continue;
            n.emplace_back(gx + dx, gy + dy);
        }
    }
    return n;
}

static inline long long hashKey(int gx, int gy) {
    return (static_cast<long long>(gx) << 32) ^ static_cast<unsigned long long>(gy);
}

std::vector<sf::Vector2f> findPathAstar(const sf::Vector2f& startWorld, const sf::Vector2f& goalWorld) {
    // Convert to grid coords
    int startGx = static_cast<int>(std::floor(startWorld.x / gridSize));
    int startGy = static_cast<int>(std::floor(startWorld.y / gridSize));
    int goalGx = static_cast<int>(std::floor(goalWorld.x / gridSize));
    int goalGy = static_cast<int>(std::floor(goalWorld.y / gridSize));

    // If goal is a wall, try to pick a nearby non-wall target (search up to radius 2)
    if (isWall(goalGx, goalGy)) {
        bool found = false;
        for (int r = 1; r <= 3 && !found; ++r) {
            for (int dx = -r; dx <= r && !found; ++dx) {
                for (int dy = -r; dy <= r && !found; ++dy) {
                    int gx = goalGx + dx;
                    int gy = goalGy + dy;
                    if (!isWall(gx, gy)) {
                        goalGx = gx;
                        goalGy = gy;
                        found = true;
                    }
                }
            }
        }
        if (!found) {
            // no reachable goal found nearby
            return {};
        }
    }

    // Open set: priority queue keyed on f = g + h
    struct PQItem {
        int gx, gy;
        float f;
        float g;
        bool operator<(PQItem const& o) const { return f > o.f; } // reversed for min-heap
    };
    std::priority_queue<PQItem> openPQ;

    // Maps for nodes
    std::unordered_map<long long, ANode> nodes;


    ANode startNode;
    startNode.g = 0.f;
    startNode.h = heuristic(startGx, startGy, goalGx, goalGy);
    startNode.parent = { INT_MIN, INT_MIN };
    nodes[hashKey(startGx, startGy)] = startNode;

    openPQ.push({ startGx, startGy, startNode.f(), startNode.g });

    std::unordered_set<long long> closed;

    const int maxIterations = 20000;
    int iter = 0;

    while (!openPQ.empty() && iter++ < maxIterations) {
        auto top = openPQ.top(); openPQ.pop();
        int cx = top.gx;
        int cy = top.gy;
        long long ckey = hashKey(cx, cy);
        if (closed.find(ckey) != closed.end()) continue;
        closed.insert(ckey);

        // reached goal?
        if (cx == goalGx && cy == goalGy) {
            // reconstruct path
            std::vector<sf::Vector2f> path;
            std::pair<int, int> cur = { cx, cy };
            while (!(cur.first == INT_MIN && cur.second == INT_MIN)) {
                path.push_back(gridToWorldCenter(cur.first, cur.second));
                long long curk = hashKey(cur.first, cur.second);
                auto it = nodes.find(curk);
                if (it == nodes.end()) break;
                cur = it->second.parent;
            }
            std::reverse(path.begin(), path.end());
            return path;
        }

        // neighbors
        for (auto nb : getNeighbors(cx, cy)) {
            int ngx = nb.first;
            int ngy = nb.second;
            long long nbk = hashKey(ngx, ngy);

            if (closed.find(nbk) != closed.end()) continue;
            if (isWall(ngx, ngy)) continue; // ignore walls

            float moveCost = heuristic(cx, cy, ngx, ngy); // diagonal cost ~= 1.414, straight 1
            float tentativeG = nodes[ckey].g + moveCost;

            auto it = nodes.find(nbk);
            if (it == nodes.end() || tentativeG < it->second.g) {
                ANode neighbor;
                neighbor.g = tentativeG;
                neighbor.h = heuristic(ngx, ngy, goalGx, goalGy);
                neighbor.parent = { cx, cy };
                nodes[nbk] = neighbor;
                openPQ.push({ ngx, ngy, neighbor.f(), neighbor.g });
            }
        }
    }

    // failed to find path
    return {};
}

void waffleCollisions(std::vector<Waffle>& waffles, float waffleRadius) {
    const int iterations = 5; 

    for (int iter = 0; iter < iterations; ++iter) {
        for (size_t i = 0; i < waffles.size(); ++i) {
            for (size_t j = i + 1; j < waffles.size(); ++j) {
                sf::Vector2f diff = waffles[j].pos - waffles[i].pos;
                float distance = std::sqrt(diff.x * diff.x + diff.y * diff.y);
                float minDistance = waffleRadius * 2.f;

                if (distance < minDistance && distance > 0.01f) {
                    // Normalize direction
                    sf::Vector2f direction(diff.x / distance, diff.y / distance);

                    // Calculate overlap
                    float overlap = minDistance - distance;

                    // Push waffles apart
                    // If one is selected and moving, let it push harder
                    bool iMoving = waffles[i].isSelected;
                    bool jMoving = waffles[j].isSelected;

                    if (iMoving && !jMoving) {
                        // i is selected, push j more
                        waffles[j].pos += direction * overlap * 0.8f;
                        waffles[i].pos -= direction * overlap * 0.2f;
                    }
                    else if (jMoving && !iMoving) {
                        // j is selected, push i more
                        waffles[i].pos -= direction * overlap * 0.8f;
                        waffles[j].pos += direction * overlap * 0.2f;
                    }
                    else {
                        // Both or neither selected, push equally
                        sf::Vector2f correction = direction * (overlap / 2.f);
                        waffles[i].pos -= correction;
                        waffles[j].pos += correction;
                    }
                }
            }
        }
    }
}

void wallCollisions(std::vector<Waffle>& waffles) {
    for (auto& w : waffles) {
        // Find which grid cell the waffle is currently in
        int centerGx = static_cast<int>(std::floor(w.pos.x / gridSize));
        int centerGy = static_cast<int>(std::floor(w.pos.y / gridSize));

        // Check the 3x3 surrounding grid cells for walls
        for (int x = -1; x <= 1; ++x) {
            for (int y = -1; y <= 1; ++y) {
                int targetGx = centerGx + x;
                int targetGy = centerGy + y;

                // If this cell is a wall (black box)
                if (isWall(targetGx, targetGy)) {
                    // Calculate World Position of the Wall Center
                    float wallX = targetGx * gridSize;
                    float wallY = targetGy * gridSize;

                    // AABB vs Circle Collision
                    // Find closest point on the square to the circle center
                    float closestX = std::max(wallX, std::min(w.pos.x, wallX + gridSize));
                    float closestY = std::max(wallY, std::min(w.pos.y, wallY + gridSize));

                    sf::Vector2f closestPoint(closestX, closestY);
                    sf::Vector2f diff = w.pos - closestPoint;

                    float distanceSq = diff.x * diff.x + diff.y * diff.y;

                    // If overlap exists (distance < radius)
                    if (distanceSq < collisionRadius * collisionRadius) {
                        float distance = std::sqrt(distanceSq);

                        // Prevent division by zero
                        if (distance > 0.001f) {
                            sf::Vector2f direction(diff.x / distance, diff.y / distance);
                            float overlap = collisionRadius - distance;

                            // Hard push out of the wall
                            w.pos += direction * overlap;
                        }
                    }
                }
            }
        }
    }
}

int main()
{
    sf::RenderWindow window(sf::VideoMode({ 1920, 1080 }), "SFML Window");
    window.setFramerateLimit(60);

    // Camera setup
    sf::View camera(sf::FloatRect({ 0.f, 0.f }, { 1920.f, 1080.f }));
    window.setView(camera);

    float cameraSpeed = 1000.f;
    float zoomLevel = 1.f;
    const float zoomSpeed = 0.1f;
    const float minZoom = 0.1f;
    const float maxZoom = 5.f;

    sf::Texture waffleTexture;
    if (!waffleTexture.loadFromFile("waffle.png")) {
        return -1;
    }

    // Create multiple waffles with spacing to avoid initial overlap
    std::vector<Waffle> waffles;
    waffles.emplace_back(sf::Vector2f(0.f, 0.f));
    waffles.emplace_back(sf::Vector2f(250.f, 0.f));
    waffles.emplace_back(sf::Vector2f(-250.f, 0.f));
    waffles.emplace_back(sf::Vector2f(0.f, 250.f));
    waffles.emplace_back(sf::Vector2f(0.f, -250.f));

    // Selection state
    bool isDragging = false;
    sf::Vector2f dragStart;
    sf::RectangleShape selectionBox;
    selectionBox.setFillColor(sf::Color(100, 100, 255, 50));
    selectionBox.setOutlineColor(sf::Color::Blue);
    selectionBox.setOutlineThickness(2.f);

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
                            if (dist <= selectionRadius) {
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
                            // compute path from waffle current pos to clicked cell
                            std::vector<sf::Vector2f> path = findPathAstar(w.pos, clickPos);
                            if (!path.empty()) {
                                w.path = std::move(path);
                                w.pathIndex = 0;
                                // set next target pos to first node in path
                                w.targetPos = w.path[w.pathIndex];
                            }
                            else {
                                // fallback: direct move if no path found
                                w.path.clear();
                                w.pathIndex = 0;
                                w.targetPos = clickPos;
                            }
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
            // If following a path, ensure targetPos is the current node
            if (!w.path.empty()) {
                // clamp pathIndex
                if (w.pathIndex >= w.path.size()) {
                    // finished path
                    w.path.clear();
                    w.pathIndex = 0;
                    // leave targetPos as is
                }
                else {
                    w.targetPos = w.path[w.pathIndex];
                }
            }

            sf::Vector2f direction = w.targetPos - w.pos;
            float distance = std::sqrt(direction.x * direction.x + direction.y * direction.y);

            if (distance > 1.f) {
                direction = sf::Vector2f(direction.x / distance, direction.y / distance);
                sf::Vector2f movement = direction * speed * deltaTime;

                if (std::sqrt(movement.x * movement.x + movement.y * movement.y) >= distance) {
                    w.pos = w.targetPos;
                    // if we just reached a path node, advance index
                    if (!w.path.empty() && w.pathIndex < w.path.size()) {
                        // arrived at node -> advance
                        w.pathIndex++;
                        if (w.pathIndex < w.path.size()) {
                            w.targetPos = w.path[w.pathIndex];
                        }
                        else {
                            // finished path; clear it
                            w.path.clear();
                            w.pathIndex = 0;
                        }
                    }
                }
                else {
                    w.pos += movement;
                }
            }
        }

        waffleCollisions(waffles, collisionRadius);
		wallCollisions(waffles);

        window.clear(sf::Color::Green);

        sf::Vector2f cameraCenter = camera.getCenter();
        sf::Vector2f cameraSize = camera.getSize();

        // Calculate which grid cells are visible on screen
        int startGx = static_cast<int>(std::floor((cameraCenter.x - cameraSize.x / 2.f) / gridSize));
        int endGx = static_cast<int>(std::ceil((cameraCenter.x + cameraSize.x / 2.f) / gridSize));
        int startGy = static_cast<int>(std::floor((cameraCenter.y - cameraSize.y / 2.f) / gridSize));
        int endGy = static_cast<int>(std::ceil((cameraCenter.y + cameraSize.y / 2.f) / gridSize));

        sf::RectangleShape cellShape;
        cellShape.setSize({ gridSize, gridSize });

        for (int x = startGx; x < endGx; ++x) {
            for (int y = startGy; y < endGy; ++y) {
                cellShape.setPosition(sf::Vector2(x * gridSize, y * gridSize));

                if (isWall(x, y)) {
                    // Draw Black Wall
                    cellShape.setFillColor(sf::Color::Black);
                    cellShape.setOutlineThickness(0);
                    window.draw(cellShape);
                }
                else {
                    // Draw Empty Green Grid Cell
                    cellShape.setFillColor(sf::Color::Transparent);
                    cellShape.setOutlineColor(sf::Color(0, 150, 0));
                    cellShape.setOutlineThickness(-2.f * zoomLevel);
                    window.draw(cellShape);
                }
            }
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
            // Draw red line if selected and moving
            if (w.isSelected) {
                sf::Vertex line[] = {
                    sf::Vertex(w.pos, sf::Color::Red),
                    sf::Vertex(w.targetPos, sf::Color::Red)
                };
                window.draw(line, 10, sf::PrimitiveType::Lines);
            }

            // Draw waffle sprite
            sf::Sprite waffleSprite(waffleTexture);
            waffleSprite.setScale(sf::Vector2f(0.25f, 0.25f));
            sf::FloatRect bounds = waffleSprite.getLocalBounds();
            waffleSprite.setOrigin(bounds.size / 2.f);
            waffleSprite.setPosition(w.pos);
            window.draw(waffleSprite);

            // Draw selection circle if selected
            if (w.isSelected) {
                sf::CircleShape selectionCircle(selectionRadius);
                selectionCircle.setOrigin(sf::Vector2(selectionRadius, selectionRadius));
                selectionCircle.setPosition(w.pos);
                selectionCircle.setFillColor(sf::Color::Transparent);
                selectionCircle.setOutlineColor(sf::Color::Blue);
                selectionCircle.setOutlineThickness(5.f * zoomLevel);
                window.draw(selectionCircle);
            }
        }

        window.display();
    }

    return 0;
}