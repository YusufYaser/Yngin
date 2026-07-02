#include <Yngin/Physics/PhysicsEngine.h>
#include "Physics_Internal.h"
#include <Yngin/Components/Collider.h>
#include "../Components/Components_Internal.h"
#include <Yngin/Core/Scenes.h>
#include <Yngin/Core/GameObject.h>
#include "../Core/Scenes/Scenes_Internal.h"
#include "../Core/GameObject/GameObject_Internal.h"
#include <glm/gtc/matrix_transform.hpp>
#include <Yngin/Rendering/Cameras.h>

namespace Yngin {
	namespace Physics {
		constexpr float SMALLEST_UNIT = 0.0001f;

		PhysicsEngine::PhysicsEngine(Context* ctx) {
			impl = std::make_unique<Impl>();

			impl->ctx = ctx;
			impl->owner = this;
		}

		PhysicsEngine::~PhysicsEngine() = default;

		Context* PhysicsEngine::getContext() const {
			return impl->ctx;
		}

		bool PhysicsEngine::isSimulationEnabled() const {
			return impl->simulationEnabled;
		}

		void PhysicsEngine::setSimulationEnabled(bool enabled) {
			impl->simulationEnabled = enabled;
		}

		float PhysicsEngine::getSimulationDistance() {
			return impl->simulationDistance;
		}

		void PhysicsEngine::setSimulationDistance(float distance) {
			impl->simulationDistance = distance;
		}

		Components::Collider* PhysicsEngine::raycast(Scene* scene, const Ray& ray, float maxDistance) const {
			if (scene->getContext() != impl->ctx) return nullptr;

			float closestDist = std::numeric_limits<float>::infinity();
			Components::Collider* closestCollider = nullptr;

			for (auto& kvp : scene->impl->gameObjectsManager->impl->gameObjects) {
				GameObject* obj = kvp.second;

				// TODO: use getComponent<Components::Collider>() when implemented
				Components::Collider* collider = obj->getComponent<Components::BoxCollider>();
				if (collider) {
					float dist = getRayIntersection(collider, ray);
					if (dist < maxDistance && dist < closestDist) {
						closestCollider = collider;
						closestDist = dist;
					}
				}
			}

			return closestCollider;
		}

		bool PhysicsEngine::checkCollision(const Components::Collider* a, const Components::Collider* b, bool fast) const {
			if (dynamic_cast<const Components::Component*>(a)->impl->gameObject->getScene() != dynamic_cast<const Components::Component*>(b)->impl->gameObject->getScene()) {
				return false;
			}

			const Components::Component* componentA = dynamic_cast<const Components::Component*>(a);
			const Components::Component* componentB = dynamic_cast<const Components::Component*>(b);
			if (componentA->impl->ctx != componentB->impl->ctx) {
				return false;
			}

			if (a->getColliderType() == Components::COLLIDER_TYPE::NONE || b->getColliderType() == Components::COLLIDER_TYPE::NONE) return false;

			if (a->getColliderType() == Components::COLLIDER_TYPE::BOX && b->getColliderType() == Components::COLLIDER_TYPE::BOX) {
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

		bool PhysicsEngine::isPointInCollider(const Components::Collider* coll, glm::vec3 point) const {
			switch (coll->getColliderType()) {
			case Components::COLLIDER_TYPE::BOX:
			{
				const Components::BoxCollider* box = dynamic_cast<const Components::BoxCollider*>(coll);
				AABBBounds aabbBounds = box->impl->getBounds();

				return
					(point.x >= aabbBounds.min.x && point.x <= aabbBounds.max.x) &&
					(point.y >= aabbBounds.min.y && point.y <= aabbBounds.max.y) &&
					(point.z >= aabbBounds.min.z && point.z <= aabbBounds.max.z);
			}
			}
			return false;
		}

		float PhysicsEngine::getRayIntersection(const Components::Collider* coll, const Ray& ray) const {
			switch (coll->getColliderType()) {
			case Components::COLLIDER_TYPE::BOX:
			{
				const Components::BoxCollider* box = dynamic_cast<const Components::BoxCollider*>(coll);
				AABBBounds aabb = box->impl->getBounds();

				glm::vec3 min = (aabb.min - ray.origin) / ray.direction;
				glm::vec3 max = (aabb.max - ray.origin) / ray.direction;

				float near = std::max({ std::min(min.x, max.x), std::min(min.y, max.y), std::min(min.z, max.z) });
				float far = std::min({ std::max(min.x, max.x), std::max(min.y, max.y), std::max(min.z, max.z) });

				if (near <= far && far >= 0) {
					return near;
				}

				return std::numeric_limits<float>::infinity();
			}
			}
			return std::numeric_limits<float>::infinity();
		}

		void PhysicsEngine::Impl::updatePhysics(Scene* scene) {
			if (!simulationEnabled) return;
			if (scene->getContext() != ctx) return;

			float t = std::min((float)ctx->getDeltaTime(), 1.0f / 15);

			std::vector<Components::RigidBody*> rigidBodies;
			std::vector<Components::Collider*> colliders;

			glm::vec3 pos = scene->getCamerasManager()->getBlendedCamera()->getPosition();

			for (auto& kvp : scene->impl->gameObjectsManager->impl->gameObjects) {
				GameObject* obj = kvp.second;

				glm::vec3 delta = obj->impl->pos - pos;
				float distSq = glm::dot(delta, delta);

				if (distSq > simulationDistance * simulationDistance) continue;

				Components::RigidBody* rigidBody = obj->getComponent<Components::RigidBody>();
				if (rigidBody) rigidBodies.push_back(rigidBody);

				// TODO: use getComponent<Components::Collider>() when implemented
				Components::Collider* collider = obj->getComponent<Components::BoxCollider>();
				if (collider) colliders.push_back(collider);
			}

			// TOOD: add rotation when hit from the side
			for (auto& rigidBody : rigidBodies) {
				GameObject* obj = rigidBody->getGameObject();

				float mass = rigidBody->impl->mass;

				rigidBody->impl->velocity.z -= scene->impl->gravity * t;

				rigidBody->impl->velocity += rigidBody->impl->impulseForceAccumulation / mass;
				rigidBody->impl->impulseForceAccumulation = glm::vec3();

				for (auto& force : rigidBody->impl->forces) {
					float time = std::min(t, force.w);
					if (force.w <= 1e-6) continue;

					rigidBody->impl->velocity += (force / mass) * time;

					force.w -= time;
				}

				glm::vec3 velocityDir = {
					rigidBody->impl->velocity.x == 0 ? 0 : rigidBody->impl->velocity.x / abs(rigidBody->impl->velocity.x),
					rigidBody->impl->velocity.y == 0 ? 0 : rigidBody->impl->velocity.y / abs(rigidBody->impl->velocity.y),
					rigidBody->impl->velocity.z == 0 ? 0 : rigidBody->impl->velocity.z / abs(rigidBody->impl->velocity.z),
				};

				obj->setPosition(obj->getPosition() + velocityDir * SMALLEST_UNIT);

				// TODO: use getComponent<Components::Collider>() when implemented
				Components::BoxCollider* a = obj->getComponent<Components::BoxCollider>();
				if (a) {
					for (auto& b : colliders) {
						if (a == b) continue;
						if (!owner->checkCollision(a, b)) continue;

						// not the physical weight
						float weight = 1.0f;

						GameObject* otherObj = b->getGameObject();
						Components::RigidBody* otherRigidBody = otherObj->getComponent<Components::RigidBody>();
						if (otherRigidBody) {
							weight = rigidBody->impl->mass / (rigidBody->impl->mass + otherRigidBody->impl->mass);
						}

						AABBBounds ab = a->impl->getBounds();
						AABBBounds bb = dynamic_cast<Components::BoxCollider*>(b)->impl->getBounds();

						glm::vec3 positive = {
							bb.max.x - ab.min.x,
							bb.max.y - ab.min.y,
							bb.max.z - ab.min.z,
						};

						glm::vec3 negative = {
							ab.max.x - bb.min.x,
							ab.max.y - bb.min.y,
							ab.max.z - bb.min.z,
						};

						glm::vec3 direction = positive;

						glm::vec3 transferedMomentum = glm::vec3();

						for (int i = 0; i < 3; i++) {
							if (negative[i] < positive[i]) direction[i] = -negative[i];
						}

						for (int i = 0; i < 3; i++) {
							int a = i;
							int b = (i + 1) % 3;
							int c = (i + 2) % 3;

							if (abs(direction[a]) < abs(direction[b]) && abs(direction[a]) < abs(direction[c])) {
								transferedMomentum[a] = rigidBody->getMomentum()[a];
								direction[b] = 0;
								direction[c] = 0;
								break;
							}
						}

						float elasticity = rigidBody->impl->elasticity;

						if (otherRigidBody) {
							elasticity = std::min(elasticity, otherRigidBody->impl->elasticity);

							rigidBody->setMomentum(rigidBody->getMomentum() - transferedMomentum * elasticity);
							otherRigidBody->setMomentum(otherRigidBody->getMomentum() + transferedMomentum * elasticity);
						} else {
							rigidBody->setMomentum(rigidBody->getMomentum() - transferedMomentum * ((rigidBody->impl->canBounce ? 1 : 0) + elasticity));
						}

						obj->impl->pos += direction * weight;
						if (otherRigidBody) {
							otherObj->impl->pos -= direction * (1 - weight);
						}
					}
				}

				obj->setPosition(obj->getPosition() + rigidBody->impl->velocity * t - velocityDir * SMALLEST_UNIT);
			}
		}
	}
}
