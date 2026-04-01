#pragma once
#include <Yngin/Core/GameObject.h>
#include <Yngin/Components/Components.h>
#include <typeindex>
#include <map>

namespace Yngin {
	struct GameObject::Impl {
		uint32_t id;

		Context* ctx;
		Scene* scene;
		GameObject* parent;

		std::map<uint32_t, std::unique_ptr<GameObject>> childs;

		glm::vec3 pos = {};
		glm::vec3 rotation = {};
		glm::vec3 scale = glm::vec3(1.0f);

		std::map<std::type_index, std::unique_ptr<Components::Component>> components;
	};

	struct GameObjectsManager::Impl {
		Context* ctx;
		Scene* scene;

		uint32_t nextId = 1;

		std::unique_ptr<GameObject> rootGameObject;
		std::map<uint32_t, GameObject*> gameObjects;
	};
}
