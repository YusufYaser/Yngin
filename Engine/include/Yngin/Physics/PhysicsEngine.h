#pragma once
#include <memory>

namespace Yngin {
	class Context;

	namespace Components {
		class Collider;
	}

	namespace Physics {
		class PhysicsEngine {
		public:
			// fast = true -> use AABB only
			// fast = false -> use SAT if AABB is true (yet to be implemented)
			bool checkCollision(const Components::Collider* a, const Components::Collider* b, bool fast = false) const;

			float getGravity();
			void setGravity(float gravity);

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
