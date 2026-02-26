#include <Yngin/Core/Context.h>
#include <glad/glad.h>
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#ifdef _WIN32
#include <windows.h>
#pragma comment(lib, "winmm.lib")
#endif

bool initialized = false;

namespace Yngin {
	bool init() {
		if (!glfwInit()) {
			terminate();
			return false;
		}

		glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
		glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
		glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef _WIN32
		timeBeginPeriod(1);
#endif

		initialized = true;

		return true;
	}

	bool isInitialized() {
		return initialized;
	}

	void terminate() {
		Context::deleteAllContexts();
		glfwTerminate();
#ifdef _WIN32
		timeEndPeriod(1);
#endif
		initialized = false;
	}
}
