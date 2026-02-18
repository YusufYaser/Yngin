#pragma once
#include <Yngin/GameObject.h>

namespace Yngin {
	struct GameObject::Impl {
		uint32_t id;

		Context* ctx;
		Scene* scene;
		GameObject* parent;
		std::map<uint32_t, std::unique_ptr<GameObject>> childs;

		glm::vec3 pos = {};
	};

	struct GameObjectsManager::Impl {
		Context* ctx;
		Scene* scene;

		uint32_t currentId = 0;

		std::unique_ptr<GameObject> rootGameObject;
		std::map<uint32_t, GameObject*> gameObjects;
	};
}
