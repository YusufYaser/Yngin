#include <Yngin/Core/Window.h>
#include <glad/glad.h>
#include "Window_Internal.h"

namespace Yngin {
	namespace {
		void fb_resize_callback(GLFWwindow* window, int width, int height) {
			GLFWwindow* oldContext = glfwGetCurrentContext();
			glfwMakeContextCurrent(window);

			Context* ctx = (Context*)glfwGetWindowUserPointer(window);

			glm::ivec4 forcedViewport = ctx->getForcedViewport();
			if (forcedViewport != glm::ivec4(0)) {
				glViewport(
					forcedViewport[0],
					height - forcedViewport[1] - forcedViewport[3],
					forcedViewport[2],
					forcedViewport[3]
				);
			} else {
				glViewport(0, 0, width, height);
			}

			glfwMakeContextCurrent(oldContext);
		}
	}

	Window::Window(Context* ctx, const WindowSettings& settings) {
		impl = std::make_unique<Impl>();
		impl->ctx = ctx;
		impl->owner = this;

		auto& m = *impl;

		m.glfwWindow = glfwCreateWindow(settings.size.x, settings.size.y, settings.title, nullptr, nullptr);
		impl->makeCurrent();
		glfwSetFramebufferSizeCallback(m.glfwWindow, fb_resize_callback);
		gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);
		glfwSetWindowUserPointer(m.glfwWindow, ctx);

		fb_resize_callback(m.glfwWindow, settings.size.x, settings.size.y);

		setFullscreen(settings.fullScreen);
		setPosition(settings.position);
	}

	Window::~Window() {
		glfwDestroyWindow(impl->glfwWindow);
	}

	void Window::setTitle(const char* title) {
		glfwSetWindowTitle(impl->glfwWindow, title);
	}

	const char* Window::getTitle() const {
		return glfwGetWindowTitle(impl->glfwWindow);
	}

	void Window::setSize(glm::ivec2 size) {
		glfwSetWindowSize(impl->glfwWindow, size.x, size.y);
	}

	glm::ivec2 Window::getSize() const {
		glm::ivec2 size;
		glfwGetWindowSize(impl->glfwWindow, &size.x, &size.y);
		return size;
	}

	void Window::maximize() {
		glfwMaximizeWindow(impl->glfwWindow);
	}

	void Window::minimize() {
		glfwRestoreWindow(impl->glfwWindow);
	}

	void Window::setPosition(glm::ivec2 pos) {
		glfwSetWindowPos(impl->glfwWindow, pos.x, pos.y);
	}

	glm::ivec2 Window::getPosition() const {
		glm::ivec2 pos;
		glfwGetWindowPos(impl->glfwWindow, &pos.x, &pos.y);
		return pos;
	}

	void Window::setCursorLocked(bool locked) {
		glfwSetInputMode(impl->glfwWindow, GLFW_CURSOR, locked ? GLFW_CURSOR_DISABLED : GLFW_CURSOR_NORMAL);
	}

	bool Window::isCursorLocked() const {
		return glfwGetInputMode(impl->glfwWindow, GLFW_CURSOR) == GLFW_CURSOR_DISABLED;
	}

	void Window::setFullscreen(bool fullscreen) {
		if (impl->fullscreen == fullscreen) return;

		impl->fullscreen = fullscreen;

		int count;
		GLFWmonitor** monitors = glfwGetMonitors(&count);
		const GLFWvidmode* vm = glfwGetVideoMode(monitors[0]);

		if (fullscreen) {
			glfwGetWindowPos(impl->glfwWindow, &impl->posPreFullscreen.x, &impl->posPreFullscreen.y);
			glfwGetWindowSize(impl->glfwWindow, &impl->sizePreFullscreen.x, &impl->sizePreFullscreen.y);
		}

		glfwSetWindowMonitor(impl->glfwWindow, fullscreen ? glfwGetPrimaryMonitor() : NULL, 0, 0, vm->width, vm->height, vm->refreshRate);

		if (!fullscreen) {
			glfwSetWindowPos(impl->glfwWindow, impl->posPreFullscreen.x, impl->posPreFullscreen.y);
			glfwSetWindowSize(impl->glfwWindow, impl->sizePreFullscreen.x, impl->sizePreFullscreen.y);
		}
	}

	bool Window::isFullscreen() const {
		return impl->fullscreen;
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

	void Window::Impl::callSizeUpdateCallback() {
		glm::ivec2 size = owner->getSize();
		fb_resize_callback(
			glfwWindow,
			size.x,
			size.y
		);
	}
}
