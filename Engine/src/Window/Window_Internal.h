#pragma once
#include <Yngin/Yngin.h>
#include <Yngin/Window.h>
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

namespace Yngin {
	struct Window::Impl {
		Context* ctx;
		GLFWwindow* glfwWindow;

		Window* owner;

		void swapBuffers();
		bool shouldClose();
		void update();
		void makeCurrent();
	};
}
