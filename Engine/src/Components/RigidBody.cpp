#include <Yngin/Components/RigidBody.h>
#include "Components_Internal.h"

namespace Yngin::Components {
	RigidBody::RigidBody(GameObject* gameObject) : Component(gameObject) {
		impl = std::make_unique<Impl>();
	}

	RigidBody::~RigidBody() = default;

	void RigidBody::setMass(float mass) {
		impl->mass = mass;
	}

	float RigidBody::getMass() {
		return impl->mass;
	}

	void RigidBody::setVelocity(glm::vec3 velocity) {
		impl->velocity = velocity;
	}

	glm::vec3 RigidBody::getVelocity() {
		return impl->velocity;
	}

	void RigidBody::applyImpulseForce(glm::vec3 force) {
		impl->impulseForceAccumulation += force;
	}

	void RigidBody::applyForce(glm::vec3 force, float time) {
		impl->forces.push_back(glm::vec4(force, time));
	}
}