#pragma once
#include <memory>
#include <glm/vec2.hpp>

namespace Yngin {
	enum class MOUSE_BUTTON : uint8_t {
		LEFT,
		RIGHT,
		MIDDLE
	};

	class InputSystem {
	public:
		// get mouse position relative to the window position
		glm::ivec2 getMousePos();

		bool isMousePressed(const MOUSE_BUTTON& button);
		bool isMouseJustPressed(const MOUSE_BUTTON& button);

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
