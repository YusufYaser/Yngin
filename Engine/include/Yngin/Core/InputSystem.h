#pragma once
#include <memory>
#include <glm/vec2.hpp>

namespace Yngin {
	enum class MOUSE_BUTTON : uint8_t {
		LEFT,
		RIGHT,
		MIDDLE
	};

	enum class KEY {
		UNKNOWN = 0,

		A, B, C, D, E, F, G,
		H, I, J, K, L, M, N,
		O, P, Q, R, S, T,
		U, V, W, X, Y, Z,

		NUM_0, NUM_1, NUM_2, NUM_3, NUM_4,
		NUM_5, NUM_6, NUM_7, NUM_8, NUM_9,

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
		glm::ivec2 getMousePos();

		bool isMousePressed(const MOUSE_BUTTON& button);
		bool isMouseJustPressed(const MOUSE_BUTTON& button);

		bool isKeyPressed(const KEY& key);
		bool isKeyJustPressed(const KEY& key);

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
