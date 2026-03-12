#pragma once
#include <Yngin/Core/Context.h>
#include <Yngin/Core/Window.h>
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

namespace Yngin {
	struct Window::Impl {
		Context* ctx;
		GLFWwindow* glfwWindow;

		Window* owner;

		glm::ivec2 posPreFullscreen;
		glm::ivec2 sizePreFullscreen;
		bool fullscreen = false;

		void swapBuffers();
		bool shouldClose();
		void update();
		void makeCurrent();

		void callSizeUpdateCallback();
	};
}
