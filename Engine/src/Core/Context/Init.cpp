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
		if (initialized) return true;

		DEBUG("Initializing Yngin");

		if (!glfwInit()) {
			DEBUG("Failed to initialize GLFW");
			terminate();
			return false;
		}

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
		if (!initialized) return;

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
