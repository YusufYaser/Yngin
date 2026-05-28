#include <Yngin/Components/Light.h>
#include "Components_Internal.h"

namespace Yngin::Components {
	Light::Light(GameObject* gameObject) : Component(gameObject) {
		impl = std::make_unique<Impl>();
	}

	Light::~Light() = default;

	LIGHT_TYPE Light::getType() const {
		return LIGHT_TYPE::NONE;
	}

	void Light::setColor(glm::vec3 color) {
		impl->color = color;
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


	PointLight::PointLight(GameObject* gameObject) : Light(gameObject) {
		impl = std::make_unique<Impl>();
	}

	PointLight::~PointLight() = default;

	LIGHT_TYPE PointLight::getType() const {
		return LIGHT_TYPE::POINT;
	}

	float PointLight::getDistance() const {
		return impl->distance;
	}

	void PointLight::setDistance(float distance) {
		impl->distance = distance;
	}
}
