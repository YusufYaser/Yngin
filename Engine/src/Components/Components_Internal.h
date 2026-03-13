#pragma once
#include <Yngin/Components/Components.h>
#include <Yngin/Components/Mesh.h>
#include <glm/vec3.hpp>

namespace Yngin {
	struct AABBBounds {
		glm::vec3 min = glm::vec3(-0.5f);
		glm::vec3 max = glm::vec3(0.5f);
	};

	namespace Components {
		struct Component::Impl {
			Context* ctx;
			GameObject* gameObject;
		};

		struct Mesh::Impl {
			uint32_t modelId;
			uint32_t texId;

			glm::vec3 color = glm::vec3(1.0f);
		};

		struct Light::Impl {
			glm::vec3 color = glm::vec3(1.0f);
			float distance = 12.0f;
			float intensity = 1.0f;
		};

		struct BoxCollider::Impl {
			BoxCollider* owner;

			glm::vec3 size = glm::vec3(1.0f);
			glm::vec3 offset = glm::vec3(0.0f);

			AABBBounds getBounds();
		};

		struct RigidBody::Impl {
			float mass = 1.0f;
			float elasticity = 0.5f;
			bool canBounce = false;

			glm::vec3 velocity;

			glm::vec3 impulseForceAccumulation;

			// xyz time
			std::vector<glm::vec4> forces;
		};
	}
}