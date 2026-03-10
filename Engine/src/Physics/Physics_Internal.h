#pragma once
#include <Yngin/Physics/Physics.h>

namespace Yngin {
	class Scene;

	namespace Physics {
		struct PhysicsEngine::Impl {
			Context* ctx;

			float gravity = 9.8f;

			void updatePhysics(Scene* scene);
		};
	}
}
