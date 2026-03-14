#pragma once
#include <Yngin/Physics/Physics.h>

namespace Yngin {
	class Scene;

	namespace Physics {
		struct PhysicsEngine::Impl {
			Context* ctx;
			PhysicsEngine* owner;

			bool simulationEnabled = true;
			float simulationDistance = 256.0f;

			void updatePhysics(Scene* scene);
		};
	}
}
