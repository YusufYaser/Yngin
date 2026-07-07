#include <Yngin/Components/Collider.h>
#include <Yngin/Core/GameObject.h>
#include <Yngin/Physics/PhysicsEngine.h>
#include "Components_Internal.h"
#include <glm/vec2.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "../Core/GameObject/GameObject_Internal.h"
#include "../Physics/Physics_Internal.h"

namespace Yngin::Components {
	Collider::Collider(GameObject* gameObject) : Component(gameObject) {}

	Collider::~Collider() {
		impl->gameObject->impl->ctx->getPhysicsEngine()->impl->deleteObject(impl->gameObject);
	}

	COLLIDER_TYPE Collider::getColliderType() const {
		return COLLIDER_TYPE::NONE;
	}

	BoxCollider::BoxCollider(GameObject* gameObject) : Collider(gameObject) {
		impl = std::make_unique<Impl>();

		impl->owner = this;
	}

	BoxCollider::~BoxCollider() = default;

	COMPONENT_TYPE BoxCollider::getType() const {
		return COMPONENT_TYPE::BOX_COLLIDER;
	}

	COLLIDER_TYPE BoxCollider::getColliderType() const {
		return COLLIDER_TYPE::BOX;
	}

	void BoxCollider::setSize(glm::vec3 size) {
		impl->size = glm::max(size, glm::vec3(0.01f));
	}

	glm::vec3 BoxCollider::getSize() {
		return impl->size;
	}

	void BoxCollider::setOffset(glm::vec3 offset) {
		impl->offset = offset;
	}

	glm::vec3 BoxCollider::getOffset() {
		return impl->offset;
	}

	AABBBounds BoxCollider::Impl::getBounds() {
		GameObject* obj = owner->getGameObject();

		glm::mat4 model = glm::mat4(1.0f);

		model = glm::translate(model, obj->getPosition() + offset);
		model = glm::rotate(model, obj->getRotation().x, glm::vec3(1, 0, 0));
		model = glm::rotate(model, obj->getRotation().y, glm::vec3(0, 1, 0));
		model = glm::rotate(model, obj->getRotation().z, glm::vec3(0, 0, 1));
		model = glm::scale(model, size * obj->getScale());

		glm::vec3 corners[8] = {
			{-0.5, -0.5, -0.5}, {0.5, -0.5, -0.5}, {-0.5, 0.5, -0.5}, {0.5, 0.5, -0.5},
			{-0.5, -0.5, 0.5}, {0.5, -0.5, 0.5}, {-0.5, 0.5, 0.5}, {0.5, 0.5, 0.5}
		};

		glm::vec3 min = glm::vec3(std::numeric_limits<float>::max());
		glm::vec3 max = glm::vec3(std::numeric_limits<float>::lowest());

		for (int i = 0; i < 8; i++) {
			glm::vec3 pos = model * glm::vec4(corners[i], 1.0f);
			min = glm::min(min, pos);
			max = glm::max(max, pos);
		}

		return AABBBounds{
			.min = min,
			.max = max
		};
	}
}
