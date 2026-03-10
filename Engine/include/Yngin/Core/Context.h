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

	namespace UI {
		class UIManager;
	}

	namespace Physics {
		class PhysicsEngine;
	}

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

		CONTEXT_STATUS getStatus() const;
		// this should be at the end of your loop
		void update();

		uint64_t getFrame() const;

		int getMaxFPS() const;
		// -1 for max, 0 for vsync
		void setMaxFPS(int newMaxFPS);

		double getFrameStartTime() const;
		double getTime();
		double getDeltaTime() const;

		Window* getWindow() const;
		glm::ivec2 getViewportSize() const;

		// TODO: replace these with getManager<T>()
		ModelsManager* getModelsManager() const;
		ScenesManager* getScenesManager() const;
		TexturesManager* getTexturesManager() const;
		ShadersManager* getShadersManager() const;
		UI::UIManager* getGlobalUIManager() const;

		Physics::PhysicsEngine* getPhysicsEngine() const;
		InputSystem* getInputSystem() const;

		template <typename T>
		T* getService() const;

		// TODO: implement internal models
		Model* getSkyboxModel() const;
		Model* getImageModel() const;

	private:
		static std::vector<Context*> contexts;
		void cleanup();

		struct Impl;
		std::unique_ptr<Impl> impl;
	};

	Context* createContext(const ContextSettings& settings = {});
}
