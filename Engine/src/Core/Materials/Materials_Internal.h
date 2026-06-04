#pragma once
#include <Yngin/Core/Materials.h>
#include <map>
#include <optional>

namespace Yngin {
	constexpr size_t MAX_MATERIALS = std::numeric_limits<uint16_t>::max() + 1;

	struct Material::Impl {
		Context* ctx;

		uint32_t id;
		glm::vec3 ambientColor = glm::vec3(1.0f);
		glm::vec3 diffuseColor = glm::vec3(1.0f);
		glm::vec3 specularColor = glm::vec3(1.0f);
		float specularComponent = 32.0f;
	};

	struct MaterialsManager::Impl {
		Context* ctx;

		size_t loadedMaterials = 0;
		std::unique_ptr<Material> materials[MAX_MATERIALS];

		std::optional<uint16_t> getAvailableId();
		uint16_t nextId = 0;
		std::vector<uint16_t> deletedIds;
	};
}
