#include <Yngin/Yngin.h>
#include <glad/glad.h>
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

namespace Yngin {
	bool initialized = false;

	bool init() {
		if (!glfwInit()) {
			terminate();
			return false;
		}

		glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
		glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
		glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

		initialized = true;

		return true;
	}

	void terminate() {
		initialized = false;
		Context::deleteAllContexts();
		glfwTerminate();
	}
}
