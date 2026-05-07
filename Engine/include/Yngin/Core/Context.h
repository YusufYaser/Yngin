#pragma once
#include <Yngin/Forward.h>
#include <map>
#include <memory>
#include <vector>
#include <stdint.h>
#include <glm/vec2.hpp>
#include <glm/vec4.hpp>
#include <Yngin/Core/Window.h>
#include <Yngin/Utils/Meta.h>

namespace Yngin {
	bool initializeYngin();
	void terminateYngin();
	bool isYnginInitialized();

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

	struct PakLoadSettings {
		bool applyContextSettings = true;
	};

	struct PakGenSettings {
		bool forceContextSettings = false;
		ContextSettings forcedContextSettings;
		std::vector<std::string> ignoredMetaPrefixes;
	};

	class Context {
	public:
		static void deleteAllContexts();

		Context(const ContextSettings& settings = {});
		~Context();
		void close();

		Meta meta;

		// It is strongly recommended to call this at the start of every frame
		// if you're using multiple contexts
		void makeCurrent();

		CONTEXT_STATUS getStatus() const;

		void notReady();
		void ready();

		// Set swapBuffers to false if you're using another library like ImGui
		// but you must to call Context::swapBuffers() at the end of your frame
		void update(bool swapBuffers = true);
		// Do not call this if you didn't set swapBuffers to false in update()
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
		// pos.xy size.xy
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
		MaterialsManager* getMaterialsManager() const;

		Rendering::Renderer* getRenderer() const;
		Physics::PhysicsEngine* getPhysicsEngine() const;
		InputSystem* getInputSystem() const;

		// You should use getModelsManager() instead
		ModelsManager* getInternalModelsManager() const;

		template <typename T>
		T* getService() const;

		void pushLoadPakSettings(const PakLoadSettings& settings);
		void popLoadPakSettings();
		PakLoadSettings getCurrentLoadPakSettings() const;

		void pushGenPakSettings(const PakGenSettings& settings);
		void popGenPakSettings();
		PakGenSettings getCurrentGenPakSettings() const;

		static bool validateGamePak(const char* gamePakData, size_t size);
		void loadGamePak(const char* gamePakData, size_t size);
		std::vector<char> generateGamePak();

		static bool validateCorePak(const char* corePakData, size_t size);
		void loadCorePak(const char* corePakData, size_t size);
		std::vector<char> generateCorePak();

		static bool validateResourcesPak(const char* resourcesPakData, size_t size);
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
