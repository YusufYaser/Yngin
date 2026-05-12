#include <Yngin/Core/Context.h>
#include <glad/glad.h>
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#ifdef _WIN32
#undef APIENTRY // stops warning
#include <windows.h>
#pragma comment(lib, "winmm.lib")
#endif

#define LOGGER_NAME Initializer
#include "../../Internal/Logger.h"

namespace {
	bool initialized = false;
}

namespace Yngin {
	bool initializeYngin() {
		DEBUG("Initializing Yngin");

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

		DEBUG("Initialized Yngin");
		return true;
	}

	bool isYnginInitialized() {
		return initialized;
	}

	void terminateYngin() {
		DEBUG("Terminating Yngin");
		Context::deleteAllContexts();
		glfwTerminate();
#ifdef _WIN32
		timeEndPeriod(1);
#endif
		initialized = false;
		DEBUG("Terminated Yngin");
	}
}
