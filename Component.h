#pragma once
#include "include.h"

class GameObject; 

class Component {
public:
	GameObject* m_owner_ptr = nullptr; // Pointer to owner GameObject
	Component() = default;
	virtual ~Component() = default;
};

