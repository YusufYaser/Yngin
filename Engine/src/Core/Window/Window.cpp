#include <Yngin/Core/Window.h>
#include <glad/glad.h>
#include "Window_Internal.h"

void fb_resize_callback(GLFWwindow* window, int width, int height) {
	GLFWwindow* oldContext = glfwGetCurrentContext();
	glfwMakeContextCurrent(window);
	glViewport(0, 0, width, height);
	glfwMakeContextCurrent(oldContext);
}

namespace Yngin {
	Window::Window(Context* ctx, const WindowSettings& settings) {
		impl = std::make_unique<Impl>();
		impl->ctx = ctx;
		impl->owner = this;

		auto& m = *impl;

		m.glfwWindow = glfwCreateWindow(settings.width, settings.height, settings.title, nullptr, nullptr);
		impl->makeCurrent();
		glfwSetFramebufferSizeCallback(m.glfwWindow, fb_resize_callback);
		gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);

		fb_resize_callback(m.glfwWindow, settings.width, settings.height);
	}

	Window::~Window() {
		glfwDestroyWindow(impl->glfwWindow);
	}

	void Window::setTitle(const char* title) {
		glfwSetWindowTitle(impl->glfwWindow, title);
	}

	void Window::setSize(int width, int height) {
		glfwSetWindowSize(impl->glfwWindow, width, height);
	}

	void Window::setPosition(int x, int y) {
		glfwSetWindowPos(impl->glfwWindow, x, y);
	}

	void Window::setCursorLocked(bool locked) {
		glfwSetInputMode(impl->glfwWindow, GLFW_CURSOR, locked ? GLFW_CURSOR_DISABLED : GLFW_CURSOR_NORMAL);
	}

	void Window::Impl::update() {
		ctx->makeCurrent();
		glfwPollEvents();
	}

	void Window::Impl::swapBuffers() {
		glfwSwapBuffers(glfwWindow);
	}

	bool Window::Impl::shouldClose() {
		return glfwWindowShouldClose(glfwWindow);
	}

	void Window::Impl::makeCurrent() {
		if (glfwGetCurrentContext() != glfwWindow) {
			glfwMakeContextCurrent(glfwWindow);
		}
	}
}
