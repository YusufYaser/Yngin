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

	float Light::getDistance() const {
		return impl->distance;
	}

	void Light::setDistance(float distance) {
		impl->distance = distance;
	}

	float Light::getIntensity() const {
		return impl->intensity;
	}

	void Light::setIntensity(float intensity) {
		impl->intensity = intensity;
	}

	glm::vec3 Light::getColor() const {
		return impl->color;
	}
}
