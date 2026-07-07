#pragma once
#include <Yngin/Forward.h>
#include <memory>
#include <glm/vec3.hpp>

namespace Yngin {
	namespace Physics {
		struct Ray {
			glm::vec3 origin = {};
			// The Ray direction must be normalized
			glm::vec3 direction = { 1.0f, 0, 0 };
		};

		class PhysicsEngine {
		public:
			Context* getContext() const;

			bool isSimulationEnabled() const;
			void setSimulationEnabled(bool enabled);

			float getSimulationDistance();
			// This (for now) does nothing
			void setSimulationDistance(float distance);

			Components::Collider* raycast(Scene* scene, const Ray& ray, float maxDistance = 1000.0f) const;

		private:
			friend class Context;
			friend struct std::default_delete<PhysicsEngine>;
			friend class Components::Collider;
			friend class Components::RigidBody;
			friend class GameObject;
			friend class ScenesManager;

			PhysicsEngine(Context* ctx);
			~PhysicsEngine();

			struct Impl;
			std::unique_ptr<Impl> impl;
		};
	}
}
