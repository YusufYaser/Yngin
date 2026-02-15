#pragma once
#include <vector>

class GLFWwindow;

namespace Yngin {
	extern bool initialized;

	bool init();
	void terminate();

	class Scene;

	class Context {
	public:
		Context();
		~Context();
		static void deleteAllContexts();

		void makeCurrent();

		void updateWindow();
		bool windowShouldClose();
		void swapBuffers();

		void addScene(Scene* scene);

	private:
		static std::vector<Context*> contexts;

		GLFWwindow* glfwWindow;

		std::vector<Scene*> scenes;
	};

	Context* createContext();
}
