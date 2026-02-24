#pragma once
#include <Yngin/GameObject.h>
#include <Yngin/Components/Components.h>
#include <typeindex>

namespace Yngin {
	struct GameObject::Impl {
		uint32_t id;

		Context* ctx;
		Scene* scene;
		GameObject* parent;

		std::map<uint32_t, std::unique_ptr<GameObject>> childs;

		glm::vec3 pos = {};
		glm::vec3 rotation = {};

		std::map<std::type_index, std::unique_ptr<Components::Component>> components;
	};

	struct GameObjectsManager::Impl {
		Context* ctx;
		Scene* scene;

		uint32_t nextId = 0;

		std::unique_ptr<GameObject> rootGameObject;
		std::map<uint32_t, GameObject*> gameObjects;
	};
}
