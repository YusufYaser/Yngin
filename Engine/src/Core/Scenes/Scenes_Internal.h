#pragma once
#include <Yngin/Core/Context.h>
#include <Yngin/Core/GameObject.h>
#include <Yngin/Core/Scenes.h>

namespace Yngin {
	class CamerasManager;
	class GameObjectsManager;

	struct Scene::Impl {
		void init();
		Scene* owner;

		uint32_t id;
		Context* ctx;

		Texture* skyboxTex;

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