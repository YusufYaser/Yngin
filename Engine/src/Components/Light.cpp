#include <Yngin/Components/Light.h>
#include "Components_Internal.h"

namespace Yngin::Components {
	Light::Light(GameObject* gameObject) : Component(gameObject) {
		impl = std::make_unique<Impl>();
	}

	Light::~Light() = default;

	LIGHT_TYPE Light::getLightType() const {
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


	// Point Light

	PointLight::PointLight(GameObject* gameObject) : Light(gameObject) {
		impl = std::make_unique<Impl>();
	}

	PointLight::~PointLight() = default;

	COMPONENT_TYPE PointLight::getType() const {
		return COMPONENT_TYPE::POINT_LIGHT;
	}

	LIGHT_TYPE PointLight::getLightType() const {
		return LIGHT_TYPE::POINT;
	}

	float PointLight::getDistance() const {
		return impl->distance;
	}

	void PointLight::setDistance(float distance) {
		impl->distance = distance;
	}


	// Directional Light

	DirectionalLight::DirectionalLight(GameObject* gameObject) : Light(gameObject) {
		impl = std::make_unique<Impl>();
	}

	DirectionalLight::~DirectionalLight() = default;

	COMPONENT_TYPE DirectionalLight::getType() const {
		return COMPONENT_TYPE::DIRECTIONAL_LIGHT;
	}

	LIGHT_TYPE DirectionalLight::getLightType() const {
		return LIGHT_TYPE::DIRECTIONAL;
	}
}
