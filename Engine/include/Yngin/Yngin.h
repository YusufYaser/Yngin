#pragma once
#include <map>
#include <memory>
#include <vector>
#include <stdint.h>

struct GLFWwindow;

namespace Yngin {
	bool init();
	void terminate();
	bool isInitialized();

	class ModelsManager;
	class Model;
	class ScenesManager;
	class Scene;
	struct Vertex;

	class Context {
	public:
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
