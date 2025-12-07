#pragma once
#include "include.h"

class Component {
public:
    GameObject* m_owner_ptr = nullptr; // Pointer to owner GameObject
    Component() = default;
    virtual ~Component() = default;
};

class Waffle {
    public:
        float x, y;
        float movementSpeed;

        Waffle() {
        
        };
        virtual ~Waffle() = default;




};