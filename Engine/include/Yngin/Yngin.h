#pragma once
#include <map>
#include <memory>
#include <vector>
#include <stdint.h>
#include <glm/vec2.hpp>

struct GLFWwindow;

namespace Yngin {
	bool init();
	void terminate();
	bool isInitialized();

	class ModelsManager;
	class ScenesManager;
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
		glm::ivec2 getViewportSize();

		ModelsManager* getModelsManager();
		ScenesManager* getScenesManager();


	private:
		static std::vector<Context*> contexts;

		void cleanup();

		GLFWwindow* glfwWindow;

		std::unique_ptr<ModelsManager> modelsManager;
		std::unique_ptr<ScenesManager> scenesManager;

		// TODO: add a custom shader class
		uint32_t shader;
	};

	Context* createContext();
}
