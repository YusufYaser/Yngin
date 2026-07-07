#include <Yngin/Components/RigidBody.h>
#include "Components_Internal.h"
#include "../Core/GameObject/GameObject_Internal.h"
#include "../Physics/Physics_Internal.h"

namespace Yngin::Components {
	RigidBody::RigidBody(GameObject* gameObject) : Component(gameObject) {
		impl = std::make_unique<Impl>();
	}

	RigidBody::~RigidBody() = default;

	COMPONENT_TYPE RigidBody::getType() const {
		return COMPONENT_TYPE::RIGID_BODY;
	}

	void RigidBody::setMass(float mass) {
		impl->mass = mass;

		if (impl->mass < 0.01f) {
			impl->mass = 0.01f;
		}

		Component::impl->gameObject->impl->physicsSync();
	}

	float RigidBody::getMass() {
		return impl->mass;
	}

	void RigidBody::setVelocity(glm::vec3 velocity) {
		impl->velocity = velocity;
		Component::impl->gameObject->impl->physicsSync();
	}

	glm::vec3 RigidBody::getVelocity() {
		return impl->velocity;
	}

	void RigidBody::setMomentum(glm::vec3 momentum) {
		impl->velocity = momentum / impl->mass;
		Component::impl->gameObject->impl->physicsSync();
	}

	glm::vec3 RigidBody::getMomentum() {
		return impl->mass * impl->velocity;
	}

	void RigidBody::setElasticity(float elasticity) {
	}

	float RigidBody::getElasticity() {
		return impl->elasticity;
	}

	void RigidBody::setCanBounce(bool canBounce) {
	}

	bool RigidBody::canBounce() {
		return impl->canBounce;
	}

	void RigidBody::applyImpulseForce(glm::vec3 force) {
	}

	void RigidBody::applyForce(glm::vec3 force, float time) {
	}

	std::vector<glm::vec4> RigidBody::getForces() {
		return impl->forces;
	}
}