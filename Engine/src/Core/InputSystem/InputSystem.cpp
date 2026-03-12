#include <Yngin/Core/InputSystem.h>
#include "InputSystem_Internal.h"
#include "../Window/Window_Internal.h"
#include <Yngin/Core/Window.h>
#include <Yngin/Core/Context.h>
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

namespace Yngin {
	namespace {
		int keyToGlfw(Yngin::KEY key) {
			if (Yngin::KEY::A <= key && key <= Yngin::KEY::Z) {
				return GLFW_KEY_A + (int(key) - int(Yngin::KEY::A));
			}

			if (Yngin::KEY::NUM_0 <= key && key <= Yngin::KEY::NUM_9) {
				return GLFW_KEY_0 + (int(key) - int(Yngin::KEY::NUM_0));
			}

			if (Yngin::KEY::F1 <= key && key <= Yngin::KEY::F25) {
				return GLFW_KEY_F1 + (int(key) - int(Yngin::KEY::F1));
			}

			if (Yngin::KEY::ESCAPE <= key && key <= Yngin::KEY::BACKSPACE) {
				return GLFW_KEY_ESCAPE + (int(key) - int(Yngin::KEY::ESCAPE));
			}

			if (Yngin::KEY::RIGHT <= key && key <= Yngin::KEY::UP) {
				return GLFW_KEY_RIGHT + (int(key) - int(Yngin::KEY::RIGHT));
			}

			if (Yngin::KEY::LSHIFT <= key && key <= Yngin::KEY::RSUPER) {
				return GLFW_KEY_LEFT_SHIFT + (int(key) - int(Yngin::KEY::LSHIFT));
			}

			if (key == Yngin::KEY::SPACE) return GLFW_KEY_SPACE;

			return 0;
		}
	}

	InputSystem::InputSystem(Context* ctx) {
		impl = std::make_unique<Impl>();

		impl->ctx = ctx;
	}

	InputSystem::~InputSystem() = default;

	void InputSystem::onUpdate() {
		uint64_t frameNum = impl->ctx->getFrame();

		for (int i = 0; i < 3; i++) {
			MOUSE_BUTTON btn = MOUSE_BUTTON(i);
			if (isMousePressed(btn)) {
				impl->lastFrameMousePressed[btn] = frameNum;
			} else {
				impl->lastFrameMouseReleased[btn] = frameNum;
			}
		}

		for (int i = 0; i < (int)Yngin::KEY::COUNT; i++) {
			KEY key = KEY(i);
			if (isKeyPressed(key)) {
				impl->lastFrameKeyPressed[key] = frameNum;
			} else {
				impl->lastFrameKeyReleased[key] = frameNum;
			}
		}
	}

	glm::ivec2 InputSystem::getMousePos(bool bypassLock) const {
		Window* window = impl->ctx->getWindow();

		if (!bypassLock && window->isCursorLocked()) return { -1, -1 };
		GLFWwindow* glfwWindow = window->impl->glfwWindow;

		double x, y;
		glfwGetCursorPos(glfwWindow, &x, &y);

		return glm::ivec2(x, y) - impl->ctx->getViewportPos();
	}

	bool InputSystem::isMousePressed(const MOUSE_BUTTON& button) const {
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

	bool InputSystem::isMouseJustPressed(const MOUSE_BUTTON& button) const {
		return impl->lastFrameMouseReleased[button] == impl->ctx->getFrame() - 2;
	}

	bool InputSystem::isMouseJustReleased(const MOUSE_BUTTON& button) const {
		return impl->lastFrameMousePressed[button] == impl->ctx->getFrame() - 2;
	}

	bool InputSystem::isKeyPressed(const KEY& key) const {
		int glfwKey = keyToGlfw(key);

		Window* window = impl->ctx->getWindow();
		GLFWwindow* glfwWindow = window->impl->glfwWindow;

		return glfwGetKey(glfwWindow, glfwKey) == GLFW_PRESS;
	}

	bool InputSystem::isKeyJustPressed(const KEY& key) const {
		return impl->lastFrameKeyReleased[key] == impl->ctx->getFrame() - 2;
	}

	bool InputSystem::isKeyJustReleased(const KEY& key) const {
		return impl->lastFrameKeyPressed[key] == impl->ctx->getFrame() - 2;
	}
}
