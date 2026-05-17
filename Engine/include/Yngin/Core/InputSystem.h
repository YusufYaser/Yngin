#pragma once
#include <Yngin/Forward.h>
#include <memory>
#include <glm/vec2.hpp>

namespace Yngin {
	enum class MOUSE_BUTTON : uint8_t {
		LEFT,
		RIGHT,
		MIDDLE,
		COUNT
	};

	enum class KEY : uint8_t {
		UNKNOWN = 0,

		A, B, C, D, E, F, G,
		H, I, J, K, L, M, N,
		O, P, Q, R, S, T,
		U, V, W, X, Y, Z,

		NUM_0, NUM_1, NUM_2, NUM_3, NUM_4,
		NUM_5, NUM_6, NUM_7, NUM_8, NUM_9,

		F1, F2, F3, F4, F5, F6, F7, F8, F9, F10, F11, F12,
		F13, F14, F15, F16, F17, F18, F19, F20, F21, F22, F23, F24, F25,

		SPACE,

		ESCAPE,
		ENTER,
		TAB,
		BACKSPACE,

		RIGHT,
		LEFT,
		DOWN,
		UP,

		LSHIFT,
		LCTRL,
		LALT,
		LSUPER,
		RSHIFT,
		RCTRL,
		RALT,
		RSUPER,


		COUNT
	};

	class InputSystem {
	public:
		// get mouse position relative to the window position
		glm::ivec2 getMousePosition(bool bypassLock = false) const;
		void setMousePosition(glm::ivec2 pos);

		bool isMousePressed(const MOUSE_BUTTON& button) const;
		bool isMouseJustPressed(const MOUSE_BUTTON& button) const;
		bool isMouseJustReleased(const MOUSE_BUTTON& button) const;

		bool isKeyPressed(const KEY& key) const;
		bool isKeyJustPressed(const KEY& key) const;
		bool isKeyJustReleased(const KEY& key) const;

	private:
		friend class Context;
		friend struct std::default_delete<InputSystem>;

		InputSystem(Context* ctx);
		~InputSystem();

		void onUpdate();

		struct Impl;
		std::unique_ptr<Impl> impl;
	};
}
