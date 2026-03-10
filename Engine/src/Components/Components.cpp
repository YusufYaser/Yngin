#include <Yngin/Components/Components.h>
#include <Yngin/Core/GameObject.h>
#include <stdexcept>
#include "Components_Internal.h"

namespace Yngin {
	namespace Components {
		Component::Component(GameObject* gameObject) {
			impl = std::make_unique<Impl>();

			impl->ctx = gameObject->getContext();
			impl->gameObject = gameObject;
		}

		Component::~Component() = default;

		GameObject* Component::getGameObject() {
			return impl->gameObject;
		}

		void Component::onRender() {}
	}
}
