#include <Yngin/Components/RigidBody.h>
#include "Components_Internal.h"

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

		if (impl->mass < Physics::SMALLEST_UNIT) {
			impl->mass = Physics::SMALLEST_UNIT;
		}
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

	void RigidBody::setMomentum(glm::vec3 momentum) {
		impl->velocity = momentum / impl->mass;
	}

	glm::vec3 RigidBody::getMomentum() {
		return impl->mass * impl->velocity;
	}

	void RigidBody::setElasticity(float elasticity) {
		impl->elasticity = elasticity;

		if (impl->elasticity < 0) {
			impl->elasticity = 0;
		} else if (impl->elasticity > 1) {
			impl->elasticity = 1;
		}
	}

	float RigidBody::getElasticity() {
		return impl->elasticity;
	}

	void RigidBody::setCanBounce(bool canBounce) {
		impl->canBounce = canBounce;
	}

	bool RigidBody::canBounce() {
		return impl->canBounce;
	}

	void RigidBody::applyImpulseForce(glm::vec3 force) {
		impl->impulseForceAccumulation += force;
	}

	void RigidBody::applyForce(glm::vec3 force, float time) {
		impl->forces.push_back(glm::vec4(force, time));
	}

	std::vector<glm::vec4> RigidBody::getForces() {
		return impl->forces;
	}
}