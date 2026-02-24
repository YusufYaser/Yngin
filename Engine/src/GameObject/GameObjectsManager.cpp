#include <Yngin/GameObject.h>
#include "GameObject_Internal.h"
#include <stdexcept>

namespace Yngin {
	GameObjectsManager::GameObjectsManager(Context* ctx, Scene* scene) {
		if (scene->getGameObjectsManager()) {
			throw std::invalid_argument("Scene already has a gameobjects manager!");
		}

		impl = std::make_unique<Impl>();

		auto& m = *impl;

		m.ctx = ctx;
		m.scene = scene;
		m.rootGameObject = std::unique_ptr<GameObject>(new GameObject(ctx, scene, nullptr));
		m.gameObjects[m.rootGameObject->getId()] = m.rootGameObject.get();
	}

	GameObjectsManager::~GameObjectsManager() {
		deleteGameObject(0);
	}

	uint32_t GameObjectsManager::acquireId() {
		return impl->nextId++;
	}

	GameObject* GameObjectsManager::getRootGameObject() {
		return impl->rootGameObject.get();
	}

	GameObject* GameObjectsManager::getGameObject(uint32_t gameObjectId) {
		auto it = impl->gameObjects.find(gameObjectId);
		assert(it != impl->gameObjects.end());

		if (it == impl->gameObjects.end()) return nullptr;

		return it->second;
	}

	void GameObjectsManager::deleteGameObject(uint32_t gameObjectId) {
		auto it = impl->gameObjects.find(gameObjectId);
		assert(it != impl->gameObjects.end());

		if (it == impl->gameObjects.end()) return;

		auto gameObject = it->second;

		impl->gameObjects.erase(gameObjectId);

		if (gameObject->getParent()) {
			gameObject->getParent()->deleteChild(gameObjectId);
		}
	}
}
