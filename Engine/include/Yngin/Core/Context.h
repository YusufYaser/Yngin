#pragma once
#include <map>
#include <memory>
#include <vector>
#include <stdint.h>
#include <glm/vec2.hpp>
#include <Yngin/Core/Window.h>

namespace Yngin {
	bool init();
	void terminate();
	bool isInitialized();

	class Window;
	class ModelsManager;
	class ScenesManager;
	class TexturesManager;
	class ShadersManager;
	struct Vertex;
	class Model;
	class InputSystem;

	enum class CONTEXT_STATUS : uint8_t {
		RUNNING,
		INITIALIZING,
		FAILED_TO_INIT,
		NEEDS_TO_STOP,
		CLEANING_UP
	};

	struct ContextSettings {
		WindowSettings windowSettings{};
	};

	class Context {
	public:
		Context(const ContextSettings& settings = {});
		~Context();
		static void deleteAllContexts();

		// it is strongly recommended to call this at the start of every frame
		// if you're using multiple contexts
		void makeCurrent();

		CONTEXT_STATUS getStatus();
		// this should be at the end of your loop
		void update();

		uint64_t getFrame();

		int getMaxFPS();
		// -1 for max, 0 for vsync
		void setMaxFPS(int newMaxFPS);

		// TODO: add startFrameTime()
		double getTime();
		double getDeltaTime();

		Window* getWindow();
		glm::ivec2 getViewportSize();

		// these will be replaced with getManager<T>() soon
		ModelsManager* getModelsManager();
		ScenesManager* getScenesManager();
		TexturesManager* getTexturesManager();
		ShadersManager* getShadersManager();

		InputSystem* getInputSystem();

		template <typename T>
		T* getService();

		// this will be removed later
		Model* getSkyboxModel();

	private:
		static std::vector<Context*> contexts;
		void cleanup();

		struct Impl;
		std::unique_ptr<Impl> impl;
	};

	Context* createContext(const ContextSettings& settings = {});
}
