#pragma once
#include <Yngin/Yngin.h>
#include <Yngin/GameObject.h>

namespace Yngin {
	class CamerasManager;
	class GameObjectsManager;

	struct Scene::Impl {
		void init();
		Scene* owner;

		uint32_t id;
		Context* ctx;
		std::unique_ptr<CamerasManager> camerasManager;
		std::unique_ptr<GameObjectsManager> gameObjectsManager;
	};

	struct ScenesManager::Impl {
		Context* ctx;
		Scene* activeScene = 0;

		uint32_t nextId = 0;

		std::map<uint32_t, std::unique_ptr<Scene>> scenes;
	};
}