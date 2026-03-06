#include <Yngin/Components/Light.h>
#include "Components_Internal.h"

namespace Yngin::Components {
	Light::Light(GameObject* gameObject) : Component(gameObject) {
		impl = std::make_unique<Impl>();
	}

	Light::~Light() = default;

	void Light::setColor(glm::vec3 color) {
		impl->color = color;
	}

	float Light::getDistance() {
		return impl->distance;
	}

	void Light::setDistance(float distance) {
		impl->distance = distance;
	}

	glm::vec3 Light::getColor() {
		return impl->color;
	}
}
