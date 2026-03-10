#include <Yngin/Physics/PhysicsEngine.h>
#include "Physics_Internal.h"
#include <Yngin/Components/Collider.h>
#include "../Components/Components_Internal.h"
#include <Yngin/Core/Scenes.h>
#include <Yngin/Core/GameObject.h>
#include "../Core/Scenes/Scenes_Internal.h"
#include "../Core/GameObject/GameObject_Internal.h"

namespace Yngin {
	namespace Physics {
		PhysicsEngine::PhysicsEngine(Context* ctx) {
			impl = std::make_unique<Impl>();

			impl->ctx = ctx;
		}

		PhysicsEngine::~PhysicsEngine() = default;

		float PhysicsEngine::getGravity() {
			return impl->gravity;
		}

		void PhysicsEngine::setGravity(float gravity) {
			impl->gravity = gravity;
		}

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
			if (scene->getContext() != ctx) return;

			float t = (float)ctx->getDeltaTime();

			for (auto& kvp : scene->impl->gameObjectsManager->impl->gameObjects) {
				GameObject* obj = kvp.second;
				Components::RigidBody* rigidBody = obj->getComponent<Components::RigidBody>();
				if (!rigidBody) continue;

				if (rigidBody->impl->mass != 0.0f) {
					float mass = rigidBody->impl->mass;

					rigidBody->impl->velocity.z -= gravity * t;

					rigidBody->impl->velocity += rigidBody->impl->impulseForceAccumulation / mass;
					rigidBody->impl->impulseForceAccumulation = glm::vec3();

					for (auto& force : rigidBody->impl->forces) {
						float time = std::min(t, force.w);
						if (force.w <= 1e-6) continue;

						rigidBody->impl->velocity += (force / mass) * time;

						force.w -= time;
					}
				}

				obj->setPosition(obj->getPosition() + rigidBody->impl->velocity * t);

				// TODO: handle collisions
			}
		}
	}
}
