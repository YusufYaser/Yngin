#pragma once
#include <Yngin/Yngin.h>

namespace Yngin {
	class Scene {
	public:
		// Do not initialize a new scene directly
		// You should use Context::createScene() instead
		Scene(Context* ctx);
		~Scene();

		Context* getContext() { return ctx; };

		void render();

	private:
		Context* ctx;
	};

	class ScenesManager {
	public:
		ScenesManager(Context* ctx);
		~ScenesManager();

		uint32_t createScene();

		void renderScene(uint32_t sceneId);

	private:
		Context* ctx;

		std::map<uint32_t, Scene*> scenes;
		uint32_t nextSceneId = 0;
	};
}
