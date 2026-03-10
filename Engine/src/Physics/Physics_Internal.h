#pragma once
#include <Yngin/Physics/Physics.h>

namespace Yngin {
	class Scene;

	namespace Physics {
		struct PhysicsEngine::Impl {
			Context* ctx;

			void updatePhysics(Scene* scene);
		};
	}
}
