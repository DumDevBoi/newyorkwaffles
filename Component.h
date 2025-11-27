#pragma once
#include "include.h"

class GameObject; 

class Component {
public:
	GameObject* m_owner_ptr = nullptr; // Pointer to owner GameObject
	void setOwner(GameObject* owner_ptr) { m_owner_ptr = owner_ptr; };
	Component() = default;
	virtual ~Component() = default;

	virtual void init() {}
	virtual void update(float deltaTime) = 0;
	virtual void draw(sf::RenderWindow& window) {};
	virtual void onDestroy() {}

	GameObject* getOwner() const { return m_owner_ptr; }
};

class MovementComponent : public Component {
public:
	sf::Vector2f target;
	float movementSpeed;
	MovementComponent(float movementSpeed = 300.f) : movementSpeed(movementSpeed) {}

	void init() override {
		GameObject* owner = getOwner();
		target = sf::Vector2f(owner->x, owner->y);
	}

	void update(float deltaTime) override {
		auto owner = getOwner();
		sf::Vector2f pos(owner->x, owner->y);
		sf::Vector2f dir = target - pos;
		float len = std::sqrt(dir.x * dir.x + dir.y * dir.y);

		if (len > 0.f) {
			dir /= len;
			owner->x += dir.x * movementSpeed * deltaTime;
			owner->y += dir.y * movementSpeed * deltaTime;
		}
	}
};

class RenderComponent : public Component {
public:
	std::shared_ptr<sf::Sprite> sprite;
	std::shared_ptr<sf::Texture> texture;

	RenderComponent(const std::string texturePath) {
		texture = std::make_shared<sf::Texture>();
		if (!texture->loadFromFile(texturePath)) {
			std::cerr << "Failed to load texture: " << texturePath << std::endl;
		}
		sprite = std::make_shared<sf::Sprite>(*texture);

		auto bounds = sprite->getLocalBounds();
		sprite->setOrigin(bounds.size / 2.0f);
	}

	void draw(sf::RenderWindow& window) override {
		window.draw(*sprite);
	}

	void update(float dt) override {
		GameObject* owner = getOwner();
		sprite->setPosition(sf::Vector2f(owner->x, owner->y));
	}
};