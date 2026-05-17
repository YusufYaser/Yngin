#pragma once
#include <Yngin/Core/InputSystem.h>

namespace Yngin {
	struct InputSystem::Impl {
		Context* ctx;

		bool mousePressedOutside[(int)Yngin::MOUSE_BUTTON::COUNT];

		uint64_t lastFrameMouseReleased[(int)Yngin::MOUSE_BUTTON::COUNT];
		uint64_t lastFrameKeyReleased[(int)Yngin::KEY::COUNT];

		uint64_t lastFrameMousePressed[(int)Yngin::MOUSE_BUTTON::COUNT];
		uint64_t lastFrameKeyPressed[(int)Yngin::KEY::COUNT];
	};
}
