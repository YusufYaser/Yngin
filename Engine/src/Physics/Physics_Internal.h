#pragma once
#include <Yngin/Physics/Physics.h>
#include <Jolt/Jolt.h>
#include <Jolt/Physics/PhysicsSystem.h>
#include <Jolt/Core/TempAllocator.h>
#include <Jolt/Core/JobSystemThreadPool.h>
#include "JoltLayers.h"

namespace Yngin {
	class Scene;

	namespace Physics {
		struct PhysicsEngine::Impl {
			Context* ctx;
			PhysicsEngine* owner;

			bool simulationEnabled = true;
			float simulationDistance = 256.0f;

			Scene* currentScene = nullptr;

			void reset();
			void setScene(Scene* scene);
			void sync(GameObject* gameObject);
			void deleteObject(GameObject* gameObject);

			void updatePhysics(Scene* scene);


			// Jolt Physics
			JPH::TempAllocatorImpl* jphTempAllocator = nullptr;
			JPH::JobSystemThreadPool* jphJobSystem = nullptr;
			JPH::PhysicsSystem* jphPhysicsSystem = nullptr;

			JoltLayers::BroadPhaseLayerInterface jphBroadPhaseInterface;
			JoltLayers::ObjectVsBroadPhaseLayerFilter jphObjectVsBPFilter;
			JoltLayers::ObjectLayerPairFilter jphObjectLayerFilter;
		};
	}
}
