#pragma once
#include <map>
#include <memory>
#include <vector>
#include <stdint.h>
#include <glm/vec2.hpp>

namespace Yngin {
	bool init();
	void terminate();
	bool isInitialized();

	class Window;
	class ModelsManager;
	class ScenesManager;
	class TexturesManager;
	struct Vertex;

	class Context {
	public:
		Context();
		~Context();
		static void deleteAllContexts();

		// it is strongly recommended to call this at the start of every frame
		// if you're using multiple contexts
		void makeCurrent();

		bool isClosing();
		// this should be at the end of your loop
		void update();

		uint64_t getFrame();

		int getMaxFPS();
		// -1 for max, 0 for vsync
		void setMaxFPS(int newMaxFPS);

		double getTime();
		double getDeltaTime();

		Window* getWindow();
		glm::ivec2 getViewportSize();

		ModelsManager* getModelsManager();
		ScenesManager* getScenesManager();
		TexturesManager* getTexturesManager();

		// will be replaced soon
		uint32_t getShaderId();

	private:
		static std::vector<Context*> contexts;
		void cleanup();

		struct Impl;
		std::unique_ptr<Impl> impl;
	};

	Context* createContext();
}
