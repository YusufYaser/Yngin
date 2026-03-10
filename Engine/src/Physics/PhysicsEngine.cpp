#include <Yngin/Physics/PhysicsEngine.h>
#include "Physics_Internal.h"
#include <Yngin/Components/Collider.h>
#include "../Components/Components_Internal.h"

namespace Yngin {
	namespace Physics {
		PhysicsEngine::PhysicsEngine(Context* ctx) {
			impl = std::make_unique<Impl>();

			impl->ctx = ctx;
		}

		PhysicsEngine::~PhysicsEngine() = default;

		bool PhysicsEngine::checkCollision(const Components::Collider* a, const Components::Collider* b, bool fast) const {
			const Components::Component* componentA = dynamic_cast<const Components::Component*>(a);
			const Components::Component* componentB = dynamic_cast<const Components::Component*>(b);
			if (componentA->impl->ctx != componentB->impl->ctx) {
				return false;
			}

			if (a->getType() == COLLIDER_TYPE::NONE || b->getType() == COLLIDER_TYPE::NONE) return false;

			if (a->getType() == COLLIDER_TYPE::BOX && b->getType() == COLLIDER_TYPE::BOX) {
				const Components::BoxCollider* boxA = dynamic_cast<const Components::BoxCollider*>(a);
				const Components::BoxCollider* boxB = dynamic_cast<const Components::BoxCollider*>(b);

				AABBBounds ab = boxA->impl->getBounds();
				AABBBounds bb = boxB->impl->getBounds();

				bool aabb =
					(ab.min.x <= bb.max.x && ab.max.x >= bb.min.x) &&
					(ab.min.y <= bb.max.y && ab.max.y >= bb.min.y) &&
					(ab.min.z <= bb.max.z && ab.max.z >= bb.min.z);

				if (!aabb) return false;

				if (fast) return true;

				// TODO: do SAT test
				return true;
			}

			return false;
		}

		void PhysicsEngine::Impl::updatePhysics(Scene* scene) {

		}
	}
}
