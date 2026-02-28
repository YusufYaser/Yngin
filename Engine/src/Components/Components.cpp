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

		void Component::onRender() {}

		Component::~Component() = default;
	}
}
