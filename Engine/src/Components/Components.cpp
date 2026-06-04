#include <Yngin/Components/Components.h>
#include <Yngin/Core/GameObject.h>
#include <Yngin/Core/Scenes.h>
#include <stdexcept>
#include "Components_Internal.h"
#include "../Core/GameObject/GameObject_Internal.h"
#include "../Core/Scenes/Scenes_Internal.h"

#define REGISTER_COMPONENT_ID(T, ID) \
template<> COMPONENT_ID getComponentId<T>() { return ID; }

namespace Yngin {
	namespace Components {
		Component::Component(GameObject* gameObject) {
			impl = std::make_unique<Impl>();

			impl->ctx = gameObject->getContext();
			impl->gameObject = gameObject;
		}

		Component::~Component() = default;

		COMPONENT_TYPE Component::getType() const {
			return COMPONENT_TYPE::NONE;
		}

		GameObject* Component::getGameObject() {
			return impl->gameObject;
		}

		void Component::onRender() {}
	}
}
