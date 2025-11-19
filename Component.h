#pragma once
#include "include.h"

class GameObject; 

class Component {
	protected:
		GameObject* m_owner = nullptr; // Pointer to the owner GameObject
		void setOwner(GameObject* owner) { m_owner = owner; }
	public:
		Component() = default; // Constructor
		virtual ~Component() = default; // Destructor
		virtual void update(float deltaTime) = 0;
		virtual void render(sf::RenderWindow& window) = 0;
		GameObject* getOwner() const { return m_owner; }
};