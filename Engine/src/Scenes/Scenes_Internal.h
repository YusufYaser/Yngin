#pragma once
#include <Yngin/Yngin.h>
#include <Yngin/GameObject.h>

namespace Yngin {
	class CamerasManager;
	class GameObjectsManager;

	struct Scene::Impl {
		Context* ctx;
		std::unique_ptr<CamerasManager> camerasManager;
		std::unique_ptr<GameObjectsManager> gameObjectsManager;
	};

	struct ScenesManager::Impl {
		Context* ctx;

		std::map<uint32_t, std::unique_ptr<Scene>> scenes;
		uint32_t nextSceneId = 0;
	};
}