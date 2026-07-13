#include <Yngin/Physics/PhysicsEngine.h>
#include "Physics_Internal.h"
#include <Yngin/Components/Components.h>
#include "../Components/Components_Internal.h"
#include <Yngin/Core/Scenes.h>
#include <Yngin/Core/GameObject.h>
#include "../Core/Scenes/Scenes_Internal.h"
#include "../Core/GameObject/GameObject_Internal.h"
#include <glm/gtc/matrix_transform.hpp>
#include <Yngin/Rendering/Cameras.h>
#include <Jolt/Jolt.h>
#include <Jolt/Physics/Body/BodyCreationSettings.h>
#include <Jolt/Physics/Body/BodyInterface.h>
#include <Jolt/Physics/Collision/Shape/BoxShape.h>
#include <Jolt/Physics/Body/BodyLockMulti.h>
#include <Jolt/Physics/Collision/RayCast.h>
#include <Jolt/Physics/Collision/CastResult.h>
#include <Jolt/Physics/Collision/Shape/RotatedTranslatedShape.h>

#define LOGGER_NAME PhysicsEngine
#include "../Internal/Logger.h"
#include <glm/gtc/quaternion.hpp>

using namespace Yngin::Components;

namespace Yngin {
	namespace Physics {
		PhysicsEngine::PhysicsEngine(Context* ctx) {
			auto& m = impl;

			m = std::make_unique<Impl>();

			m->ctx = ctx;
			m->owner = this;

			// Setup JoltPhysics
			m->jphTempAllocator = new JPH::TempAllocatorImpl(128 * 1024 * 1024);

			m->jphJobSystem = new JPH::JobSystemThreadPool(
				JPH::cMaxPhysicsJobs,
				JPH::cMaxPhysicsBarriers,
				std::thread::hardware_concurrency() - 1
			);

			m->jphPhysicsSystem = new JPH::PhysicsSystem();
			m->jphPhysicsSystem->Init(
				10240,
				0,
				10240,
				10240,
				m->jphBroadPhaseInterface,
				m->jphObjectVsBPFilter,
				m->jphObjectLayerFilter
			);
		}

		PhysicsEngine::~PhysicsEngine() {
			auto& m = impl;

			delete m->jphPhysicsSystem;
			m->jphPhysicsSystem = nullptr;

			delete m->jphJobSystem;
			m->jphJobSystem = nullptr;

			delete m->jphTempAllocator;
			m->jphTempAllocator = nullptr;
		}

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

		Components::Collider* PhysicsEngine::raycast(Scene* scene, const Ray& rayData, float maxDistance) const {
			JPH::RRayCast ray;

			ray.mOrigin = JPH::RVec3(rayData.origin.x, rayData.origin.y, rayData.origin.z);
			ray.mDirection = JPH::Vec3(rayData.direction.x, rayData.direction.y, rayData.direction.z) * maxDistance;

			JPH::RayCastResult result;

			bool hasHit = impl->jphPhysicsSystem->GetNarrowPhaseQuery().CastRay(ray, result);

			if (!hasHit) return nullptr;

			Components::Collider* coll = nullptr;

			JPH::BodyID bodyId = result.mBodyID;

			JPH::BodyInterface& bodyInterface = impl->jphPhysicsSystem->GetBodyInterface();

			auto& lockInterface = impl->jphPhysicsSystem->GetBodyLockInterface();

			{
				JPH::BodyLockRead lock(lockInterface, bodyId);
				if (lock.Succeeded()) {
					const JPH::Body& body = lock.GetBody();

					GameObject* gameObject = reinterpret_cast<GameObject*>(body.GetUserData());
					if (gameObject == nullptr) return nullptr;

					coll = gameObject->getComponent<Components::BoxCollider>();
				}
			}

			return coll;
		}

		void PhysicsEngine::Impl::reset() {
			JPH::BodyInterface& bodyInterface = jphPhysicsSystem->GetBodyInterface();

			JPH::BodyIDVector bodiesIds;
			jphPhysicsSystem->GetBodies(bodiesIds);

			std::vector<JPH::BodyID> safeToRemove;

			{
				auto& lockInterface = jphPhysicsSystem->GetBodyLockInterface();
				JPH::BodyLockMultiRead lock(lockInterface, bodiesIds.data(), static_cast<int>(bodiesIds.size()));

				for (int i = 0; i < bodiesIds.size(); i++) {
					const JPH::Body* body = lock.GetBody(i);

					if (body == nullptr) continue;

					if (!body->IsInBroadPhase()) continue;

					safeToRemove.push_back(body->GetID());
				}
			}

			bodyInterface.RemoveBodies(safeToRemove.data(), static_cast<int>(safeToRemove.size()));
		}

		void PhysicsEngine::Impl::setScene(Scene* scene) {
			if (scene == nullptr) {
				currentScene = nullptr;
				return;
			}

			if (scene->getContext() != ctx) return;

			currentScene = scene;

			jphPhysicsSystem->SetGravity(JPH::Vec3(0.0f, 0.0f, -scene->getGravity()));

			for (auto& [id, gameObject] : scene->getGameObjectsManager()->impl->gameObjects) {
				sync(gameObject);
			}
		}

		void PhysicsEngine::Impl::sync(GameObject* gameObject) {
			if (currentScene == nullptr) return;
			if (gameObject->getContext() != ctx) return;
			if (gameObject->getScene() != currentScene) return;

			JPH::BodyInterface& bodyInterface = jphPhysicsSystem->GetBodyInterface();

			JPH::Body*& body = gameObject->impl->joltBody;
			JPH::BodyID bodyId;
			if (body) bodyId = body->GetID();

			BoxCollider* coll = gameObject->getComponent<BoxCollider>();
			RigidBody* rb = gameObject->getComponent<RigidBody>();

			bool hasJoltBody = coll != nullptr;
			bool dynamic = rb != nullptr;

			if (!hasJoltBody && body != nullptr) {
				if (bodyInterface.IsAdded(bodyId)) bodyInterface.RemoveBody(bodyId);
				bodyInterface.DestroyBody(bodyId);
				body = nullptr;
			}

			if (hasJoltBody && body == nullptr) {
				JPH::BoxShapeSettings boxSettings(JPH::Vec3(1.0f, 1.0f, 1.0f));
				JPH::ShapeRefC boxShape = boxSettings.Create().Get();

				JPH::BodyCreationSettings bodySettings{
					boxShape,
					JPH::RVec3(0.0f, 0.0f, 0.0f),
					JPH::Quat::sIdentity(),
					JPH::EMotionType::Dynamic,
					JoltLayers::Layers::STATIC
				};

				bodySettings.mMotionQuality = JPH::EMotionQuality::LinearCast;

				body = bodyInterface.CreateBody(bodySettings);
				if (body == nullptr) return;

				bodyId = body->GetID();

				body->SetMotionType(JPH::EMotionType::Static);
				body->SetUserData(reinterpret_cast<uint64_t>(gameObject));

				bodyInterface.AddBody(bodyId, JPH::EActivation::Activate);
			}

			if (body == nullptr) return;

			glm::vec3 boxSize = coll->getSize() * gameObject->getScale();

			boxSize /= 2.0f;

			JPH::BoxShapeSettings boxSettings(JPH::Vec3(
				boxSize.x,
				boxSize.y,
				boxSize.z
			));

			JPH::ShapeRefC boxShape = boxSettings.Create().Get();

			JPH::Vec3 localOffset(
				coll->getOffset().x,
				coll->getOffset().y,
				coll->getOffset().z
			);

			JPH::RotatedTranslatedShapeSettings offsetShapeSettings(localOffset, JPH::Quat::sIdentity(), boxShape);
			JPH::ShapeRefC finalShape = offsetShapeSettings.Create().Get();

			bodyInterface.SetShape(bodyId, finalShape, true, JPH::EActivation::Activate);

			bodyInterface.SetMotionType(bodyId, dynamic ? JPH::EMotionType::Dynamic : JPH::EMotionType::Static, JPH::EActivation::Activate);

			bodyInterface.SetObjectLayer(bodyId, dynamic ? JoltLayers::Layers::DYNAMIC : JoltLayers::Layers::STATIC);

			bodyInterface.SetPositionAndRotation(
				bodyId,
				JPH::RVec3(
					gameObject->getPosition().x,
					gameObject->getPosition().y,
					gameObject->getPosition().z
				),
				JPH::Quat::sEulerAngles(
					JPH::Vec3(
						gameObject->getRotation().x,
						gameObject->getRotation().y,
						gameObject->getRotation().z
					)
				),
				JPH::EActivation::Activate
			);

			bodyInterface.SetLinearVelocity(
				bodyId,
				dynamic ?
				JPH::Vec3(
					rb->impl->velocity.x,
					rb->impl->velocity.y,
					rb->impl->velocity.z
				)
				:
				JPH::Vec3(0.0f, 0.0f, 0.0f)
			);
		}

		void PhysicsEngine::Impl::deleteObject(GameObject* gameObject) {
			if (gameObject->getContext() != ctx) return;

			JPH::Body*& body = gameObject->impl->joltBody;
			if (body == nullptr) return;

			JPH::BodyInterface& bodyInterface = jphPhysicsSystem->GetBodyInterface();

			if (gameObject->getScene() == currentScene) bodyInterface.RemoveBody(body->GetID());
			bodyInterface.DestroyBody(body->GetID());
			body = nullptr;
		}

		void PhysicsEngine::Impl::updatePhysics(Scene* scene) {
			if (currentScene == nullptr) return;
			if (!simulationEnabled) return;
			if (scene->getContext() != ctx) return;

			jphPhysicsSystem->Update(ctx->getDeltaTime(), 1, jphTempAllocator, jphJobSystem);

			JPH::BodyInterface& bodyInterface = jphPhysicsSystem->GetBodyInterface();

			JPH::BodyIDVector bodiesIds;
			jphPhysicsSystem->GetBodies(bodiesIds);

			{
				auto& lockInterface = jphPhysicsSystem->GetBodyLockInterface();
				JPH::BodyLockMultiRead lock(lockInterface, bodiesIds.data(), static_cast<int>(bodiesIds.size()));

				for (int i = 0; i < bodiesIds.size(); i++) {
					const JPH::Body* body = lock.GetBody(i);

					if (body == nullptr || body->IsStatic()) continue;

					GameObject* gameObject = reinterpret_cast<GameObject*>(body->GetUserData());

					if (gameObject == nullptr) continue;

					JPH::RVec3 pos = body->GetPosition();
					JPH::Quat rot = body->GetRotation();

					gameObject->impl->pos = glm::vec3(pos.GetX(), pos.GetY(), pos.GetZ());
					gameObject->impl->rotation = glm::eulerAngles(glm::quat(rot.GetW(), rot.GetX(), rot.GetY(), rot.GetZ()));
					gameObject->impl->updateMatrices = true;

					RigidBody* rb = gameObject->getComponent<RigidBody>();

					JPH::Vec3 velocity = body->GetLinearVelocity();
					rb->impl->velocity = glm::vec3(velocity.GetX(), velocity.GetY(), velocity.GetZ());
				}
			}
		}
	}
}
