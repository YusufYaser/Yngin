#pragma once
#include <Yngin/Core/Materials.h>
#include <map>

namespace Yngin {
	struct Material::Impl {
		Context* ctx;

		uint32_t id;
		glm::vec3 ambientColor;
		glm::vec3 diffuseColor;
		glm::vec3 specularColor;
		float specularComponent;
	};

	struct MaterialsManager::Impl {
		Context* ctx;

		std::map<uint32_t, std::unique_ptr<Material>> materials;
		uint32_t nextId = 0;
	};
}
