#pragma once
#include <memory>
#include "Component.h"
#include <glm/vec3.hpp>

namespace Yngin {
	namespace Components {
		class RigidBody : public Component {
		public:
			void setMass(float mass);
			float getMass();

			void setVelocity(glm::vec3 velocity);
			glm::vec3 getVelocity();

			void setMomentum(glm::vec3 momentum);
			glm::vec3 getMomentum();

			void setElasticity(float elasticity);
			float getElasticity();

			// control bouncing when hitting a collider with no RigidBody
			void setCanBounce(bool canBounce);
			bool canBounce();

			void applyImpulseForce(glm::vec3 force);
			void applyForce(glm::vec3 force, float time);


		private:
			friend class GameObject;
			friend struct std::default_delete<RigidBody>;
			friend class Physics::PhysicsEngine;

			RigidBody(GameObject* gameObject);
			~RigidBody();

			struct Impl;
			std::unique_ptr<Impl> impl;
		};
	}
}
