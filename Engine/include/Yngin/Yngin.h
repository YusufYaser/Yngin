#pragma once
#include <map>
#include <memory>
#include <vector>
#include <stdint.h>

class GLFWwindow;

namespace Yngin {
	extern bool initialized;

	bool init();
	void terminate();

	class ModelsManager;
	class Model;
	class ScenesManager;
	class Scene;
	struct Vertex;

	class Context {
	public:
		// Do not initialize a new context directly
		// You should use Yngin::createContext() instead
		Context();
		~Context();
		static void deleteAllContexts();

		void makeCurrent();

		void updateWindow();
		bool windowShouldClose();
		void swapBuffers();

		ModelsManager* getModelsManager();
		ScenesManager* getScenesManager();

	private:
		static std::vector<Context*> contexts;

		void cleanup();

		GLFWwindow* glfwWindow;

		ModelsManager* modelsManager;
		ScenesManager* scenesManager;

		// TODO: add a custom shader class
		uint32_t shader;
	};

	Context* createContext();
}
