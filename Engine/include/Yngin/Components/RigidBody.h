#pragma once
#include <Yngin/Forward.h>
#include <memory>
#include "Component.h"
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>

namespace Yngin {
	namespace Components {
		class RigidBody : public Component {
		public:
			static const COMPONENT_TYPE staticType = COMPONENT_TYPE::RIGID_BODY;
			COMPONENT_TYPE getType() const;

			void setMass(float mass);
			float getMass();

			void setVelocity(glm::vec3 velocity);
			glm::vec3 getVelocity();

			void setMomentum(glm::vec3 momentum);
			glm::vec3 getMomentum();

			// DEPRECATED: This function now does nothing and will be removed soon
			void setElasticity(float elasticity);
			// DEPRECATED: This function now does nothing and will be removed soon
			float getElasticity();

			// DEPRECATED: This function now does nothing and will be removed soon
			void setCanBounce(bool canBounce);
			// DEPRECATED: This function now does nothing and will be removed soon
			bool canBounce();

			void applyImpulseForce(glm::vec3 force);
			void applyForce(glm::vec3 force, float time);

			// Force xyz time
			std::vector<glm::vec4> getForces();

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
