#pragma once
#include "include.h"

class GameObject; 

class Component {
	protected:
		GameObject* m_owner_ptr = nullptr; // Pointer to owner GameObject
		void setOwner(GameObject* owner_ptr) { m_owner_ptr = owner_ptr; };
	public:
		Component() = default; // Constructor
		virtual ~Component() = default; // Destructor

		virtual void init() {}
		virtual void update(float deltaTime) = 0;
		virtual void render(sf::RenderWindow& window) = 0;
		virtual void onDestroy() {}

		GameObject* getOwner() const { return m_owner_ptr; }

		friend class GameObject;
};