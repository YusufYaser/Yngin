#include <Yngin/Yngin.h>
#include <glad/glad.h>
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

void fb_resize_callback(GLFWwindow* window, int width, int height) {
	GLFWwindow* oldContext = glfwGetCurrentContext();
	glfwMakeContextCurrent(window);
	glViewport(0, 0, width, height);
	glfwMakeContextCurrent(oldContext);
}

namespace Yngin {
	std::vector<Context*> Context::contexts;

	Context* createContext() {
		Context* ctx = nullptr;
		if (initialized) ctx = new Context();
		return ctx;
	}

	Context::Context() {
		if (!initialized) {
			throw std::exception("Cannot create new context before initialization");
		}
		contexts.push_back(this);

		glfwWindow = glfwCreateWindow(800, 600, "Game", nullptr, nullptr);
		glfwSetFramebufferSizeCallback(glfwWindow, fb_resize_callback);
		makeCurrent();
		gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);

		glClearColor(0, 0, 0, 1);
		glClear(GL_COLOR_BUFFER_BIT);

		glfwSwapBuffers(glfwWindow);
	}

	Context::~Context() {
		glfwDestroyWindow(glfwWindow);
		contexts.erase(std::find(contexts.begin(), contexts.end(), this));
	}

	void Yngin::Context::deleteAllContexts() {
		for (Context* ctx : contexts) {
			delete ctx;
		}
	}

	void Context::makeCurrent() {
		if (glfwGetCurrentContext() != glfwWindow) {
			glfwMakeContextCurrent(glfwWindow);
		}
	}

	void Context::updateWindow() {
		makeCurrent();

		glfwPollEvents();
	}

	bool Context::windowShouldClose() {
		return glfwWindowShouldClose(glfwWindow);
	}
}
