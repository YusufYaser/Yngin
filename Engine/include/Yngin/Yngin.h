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

		void makeCurrent();

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
