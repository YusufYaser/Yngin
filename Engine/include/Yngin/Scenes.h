#pragma once
#include <Yngin/Yngin.h>
#include <memory>

namespace Yngin {
	class CamerasManager;
	class GameObjectsManager;
	class GameObject;
	class Scene;

	class ScenesManager {
	public:
		ScenesManager(Context* ctx);
		~ScenesManager();

		Scene* getScene(uint32_t sceneId);

		uint32_t createScene();
		void deleteScene(uint32_t sceneId);

	private:
		struct Impl;
		std::unique_ptr<Impl> impl;
	};

	class Scene {
	public:
		~Scene();

		Context* getContext();

		CamerasManager* getCamerasManager();
		GameObjectsManager* getGameObjectsManager();

		void render();

	private:
		Scene(Context* ctx);
		friend class ScenesManager;

		struct Impl;
		std::unique_ptr<Impl> impl;
	};
}
