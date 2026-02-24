#include <Yngin/Components/Components.h>
#include "Components_Internal.h"

namespace Yngin {
	namespace Components {
		Component::Component(Context* ctx, GameObject* gameObject) {
			impl = std::make_unique<Impl>();

			impl->ctx = ctx;
			impl->gameObject = gameObject;
		}

		void Component::onRender() {}

		Component::~Component() = default;
	}
}
