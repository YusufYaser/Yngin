#pragma once
#include <Yngin/Core/InputSystem.h>
#include <map>

namespace Yngin {
	struct InputSystem::Impl {
		Context* ctx;

		std::map<Yngin::MOUSE_BUTTON, uint64_t> lastFrameMouseReleased;
	};
}
