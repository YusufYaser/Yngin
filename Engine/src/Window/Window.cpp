#include <Yngin/Window.h>
#include <glad/glad.h>
#include "Window_Internal.h"

void fb_resize_callback(GLFWwindow* window, int width, int height) {
	GLFWwindow* oldContext = glfwGetCurrentContext();
	glfwMakeContextCurrent(window);
	glViewport(0, 0, width, height);
	glfwMakeContextCurrent(oldContext);
}

namespace Yngin {
	Window::Window(Context* ctx) {
		impl = std::make_unique<Impl>();
		impl->ctx = ctx;
		impl->owner = this;

		auto& m = *impl;

		m.glfwWindow = glfwCreateWindow(800, 600, "Yngin Instance", nullptr, nullptr);
		impl->makeCurrent();
		glfwSetFramebufferSizeCallback(m.glfwWindow, fb_resize_callback);
		gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);

		fb_resize_callback(m.glfwWindow, 800, 600);
	}

	Window::~Window() {
		glfwDestroyWindow(impl->glfwWindow);
	}

	void Window::update() {
		impl->ctx->makeCurrent();
		glfwPollEvents();
	}

	void Window::swapBuffers() {
		glfwSwapBuffers(impl->glfwWindow);
	}

	bool Window::shouldClose() {
		return glfwWindowShouldClose(impl->glfwWindow);
	}

	void Window::Impl::makeCurrent() {
		if (glfwGetCurrentContext() != glfwWindow) {
			glfwMakeContextCurrent(glfwWindow);
		}
	}
}
