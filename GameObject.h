#pragma once
#include "Component.h"
#include "include.h"

class GameObject {
	public:
		GameObject() = default;
		virtual ~GameObject() = default;

		//addComponent
        template<typename T, typename... Args>
        T* addComponent(Args&&... args)   //perfect forwarding
        {
			//compile time 
            static_assert(std::is_base_of<Component, T>::value,
                "T must inherit from Component");


            auto component = std::make_unique<T>(std::forward<Args>(args)...);
            T* component_ptr = component.get();

            component->setOwner(this);



            component_ptr->init();

            return component_ptr;
        }
};