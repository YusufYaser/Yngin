#pragma once
#include <memory>
#include <glm/vec3.hpp>

namespace Yngin {
	class Context;
	class Scene;

	namespace Components {
		class Collider;
	}

	namespace Physics {
		constexpr float SMALLEST_UNIT = 0.1f;

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
			void setSimulationDistance(float distance);

			// fast = true -> use AABB only
			// fast = false -> use SAT if AABB is true (yet to be implemented)
			bool checkCollision(const Components::Collider* a, const Components::Collider* b, bool fast = false) const;
			bool isPointInCollider(const Components::Collider* coll, glm::vec3 point) const;
			float getRayIntersection(const Components::Collider* coll, const Ray& ray) const;

			Components::Collider* raycast(Scene* scene, const Ray& ray, float maxDistance = std::numeric_limits<float>::infinity()) const;

		private:
			friend class Context;
			friend struct std::default_delete<PhysicsEngine>;

			PhysicsEngine(Context* ctx);
			~PhysicsEngine();

			struct Impl;
			std::unique_ptr<Impl> impl;
		};
	}
}
