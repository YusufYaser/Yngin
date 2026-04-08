#include <Yngin/Core/Materials.h>
#include "Materials_Internal.h"

namespace Yngin {
	Material::Material(Context* ctx) {
		impl = std::make_unique<Impl>();

		impl->ctx = ctx;
	}

	Material::~Material() = default;

	uint32_t Material::getId() const {
		return impl->id;
	}

	Context* Material::getContext() const {
		return impl->ctx;
	}

	glm::vec3 Yngin::Material::getAmbientColor() const {
		return impl->ambientColor;
	}

	void Material::setAmbientColor(glm::vec3 color) {
		impl->ambientColor = color;
	}

	glm::vec3 Material::getDiffuseColor() const {
		return impl->diffuseColor;
	}

	void Material::setDiffuseColor(glm::vec3 color) {
		impl->diffuseColor = color;
	}

	glm::vec3 Material::getSpecularColor() const {
		return impl->specularColor;
	}

	void Material::setSpecularColor(glm::vec3 color) {
		impl->specularColor = color;
	}

	float Material::getSpecularComponent() const {
		return impl->specularComponent;
	}

	void Material::setSpecularComponent(float component) {
		impl->specularComponent = component;
	}
}
