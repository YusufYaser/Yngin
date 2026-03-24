#pragma once
#include <map>
#include <memory>
#include <vector>
#include <stdint.h>
#include <glm/vec2.hpp>
#include <glm/vec4.hpp>
#include <Yngin/Core/Window.h>

namespace Yngin {
	bool initializeYngin();
	void terminateYngin();
	bool isYnginInitialized();

	class Window;
	class ModelsManager;
	class ScenesManager;
	class TexturesManager;
	class ShadersManager;
	class ScriptsManager;
	struct Vertex;
	class Model;
	class InputSystem;

	namespace UI {
		class UIManager;
	}

	namespace Physics {
		class PhysicsEngine;
	}

	namespace Rendering {
		class Renderer;
	}

	enum class CONTEXT_STATUS : uint8_t {
		RUNNING,
		INITIALIZING,
		WAITING_FOR_READY,
		FAILED_TO_INIT,
		NEEDS_TO_STOP,
		CLEANING_UP
	};

	struct ContextSettings {
		WindowSettings windowSettings{};
	};

	namespace GameFiles {
		const int VERSION = 1;
	}

	class Context {
	public:
		Context(const ContextSettings& settings = {});
		~Context();
		void close();
		static void deleteAllContexts();

		// it is strongly recommended to call this at the start of every frame
		// if you're using multiple contexts
		void makeCurrent();

		CONTEXT_STATUS getStatus() const;

		void ready();

		// Set swapBuffers to false if you're using another library like ImGui
		// but you need to call Context::swapBuffers() at the end of your frame
		void update(bool swapBuffers = true);
		void swapBuffers();

		uint64_t getFrame() const;

		int getMaxFPS() const;
		// -1 for max, 0 for vsync
		void setMaxFPS(int newMaxFPS);

		double getFrameStartTime() const;
		double getTime();
		double getDeltaTime() const;

		Window* getWindow() const;

		void forceViewport(glm::ivec2 pos, glm::ivec2 size);
		glm::ivec4 getForcedViewport() const;
		glm::ivec2 getViewportPos() const;
		glm::ivec2 getViewportSize() const;

		// TODO: replace these with getManager<T>()
		ModelsManager* getModelsManager() const;
		ScenesManager* getScenesManager() const;
		TexturesManager* getTexturesManager() const;
		ShadersManager* getShadersManager() const;
		UI::UIManager* getGlobalUIManager() const;
		ScriptsManager* getScriptsManager() const;

		Rendering::Renderer* getRenderer() const;
		Physics::PhysicsEngine* getPhysicsEngine() const;
		InputSystem* getInputSystem() const;

		// You should use getModelsManager() instead
		ModelsManager* getInternalModelsManager() const;

		template <typename T>
		T* getService() const;

		void loadGamePak(const char* gamePakData, size_t size);
		std::vector<char> generateGamePak();

		void loadCorePak(const char* corePakData, size_t size);
		// You can replace the settings stored in the core.pak
		// By default, it uses the initial settings you used when you
		// initialized the context
		std::vector<char> generateCorePak();
		std::vector<char> generateCorePak(const ContextSettings& settings);

		void loadResourcesPak(const char* resourcesPakData, size_t size);
		std::vector<char> generateResourcesPak();

	private:
		void cleanup();
		static std::vector<Context*> contexts;

		struct Impl;
		std::unique_ptr<Impl> impl;
	};

	Context* createContext(const ContextSettings& settings = {});
}
