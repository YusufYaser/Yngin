#pragma once
#include <Yngin/Components/Components.h>
#include <Yngin/Components/Mesh.h>
#include <glm/vec3.hpp>

namespace Yngin {
	namespace Components {
		struct Component::Impl {
			Context* ctx;
			GameObject* gameObject;
		};

		struct Mesh::Impl {
			uint32_t modelId;
			uint32_t texId;

			glm::vec3 scale = glm::vec3(1.0f);

			glm::vec3 color = glm::vec3(1.0f);
		};

		struct Light::Impl {
			glm::vec3 color = glm::vec3(1.0f);
			float distance = 12.0f;
			float intensity = 1.0f;
		};
	}
}
