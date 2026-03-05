#include <Yngin/Core/InputSystem.h>
#include "InputSystem_Internal.h"
#include "../Window/Window_Internal.h"
#include <Yngin/Core/Window.h>
#include <Yngin/Core/Context.h>
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

namespace Yngin {
	InputSystem::InputSystem(Context* ctx) {
		impl = std::make_unique<Impl>();

		impl->ctx = ctx;
	}

	InputSystem::~InputSystem() = default;

	void InputSystem::onUpdate() {
		uint64_t frameNum = impl->ctx->getFrame();
		for (int i = 0; i < 3; i++) {
			MOUSE_BUTTON btn = MOUSE_BUTTON(i);
			if (!isMousePressed(btn)) {
				impl->lastFrameMouseReleased[btn] = frameNum;
			}
		}
	}

	glm::ivec2 InputSystem::getMousePos() {
		Window* window = impl->ctx->getWindow();
		GLFWwindow* glfwWindow = window->impl->glfwWindow;

		double x, y;
		glfwGetCursorPos(glfwWindow, &x, &y);

		return { x, y };
	}

	bool InputSystem::isMousePressed(const MOUSE_BUTTON& button) {
		int glfwButton = -1;
		switch (button) {
		case MOUSE_BUTTON::LEFT:
			glfwButton = GLFW_MOUSE_BUTTON_LEFT;
			break;
		case MOUSE_BUTTON::RIGHT:
			glfwButton = GLFW_MOUSE_BUTTON_RIGHT;
			break;
		case MOUSE_BUTTON::MIDDLE:
			glfwButton = GLFW_MOUSE_BUTTON_MIDDLE;
			break;
		}
		if (glfwButton == -1) return false;

		Window* window = impl->ctx->getWindow();
		GLFWwindow* glfwWindow = window->impl->glfwWindow;

		return glfwGetMouseButton(glfwWindow, glfwButton);
	}

	bool InputSystem::isMouseJustPressed(const MOUSE_BUTTON& button) {
		return impl->lastFrameMouseReleased[button] == impl->ctx->getFrame() - 2;
	}
}
