#pragma once
#include "Component.h"
#include "include.h"

class GameObject {
    public:
        GameObject() = default;
        virtual ~GameObject() = default;

        float x = 0, y = 0;

        //addComponent
        template<typename T, typename... Args>
        T* addComponent(Args&&... args)   //perfect forwarding
        {
            static_assert(std::is_base_of<Component, T>::value,"T must inherit from Component"); //compiletime
            auto component = std::make_unique<T>(std::forward<Args>(args)...);
            T* component_ptr = component.get();
            component->setOwner(this);
            component_ptr->init();
            m_components.push_back(std::move(component));
            return component_ptr;
        }

        void draw(sf::RenderWindow& window) {
            for (auto& c : m_components)
                c->draw(window);
        }

        void update(float deltaTime) {
            for (auto& c : m_components)
                c->update(deltaTime);
        }

    private:
        std::vector<std::unique_ptr<Component>> m_components;
};