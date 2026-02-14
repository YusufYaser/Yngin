#pragma once
#include <vector>

class GLFWwindow;

namespace Yngin {
	extern bool initialized;

	bool init();
	void terminate();

	class Context {
	public:
		Context();
		~Context();
		static void deleteAllContexts();

		void makeCurrent();

		void updateWindow();
		bool windowShouldClose();

	private:
		static std::vector<Context*> contexts;

		GLFWwindow* glfwWindow;
	};

	Context* createContext();
}
