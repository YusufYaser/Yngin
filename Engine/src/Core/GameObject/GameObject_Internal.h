#pragma once
#include <Yngin/Core/GameObject.h>
#include <Yngin/Components/Components.h>
#include <typeindex>
#include <map>
#include <glm/matrix.hpp>

namespace Yngin {
	enum class COMPONENT_ID : uint8_t {
		NONE = 0,
		MESH,
		POINT_LIGHT,
		BOX_COLLIDER,
		RIGID_BODY,
		COUNT
	};

	struct GameObject::Impl {
		uint32_t id;

		Context* ctx;
		Scene* scene;
		GameObject* parent;

		std::map<uint32_t, std::unique_ptr<GameObject>> childs;

		glm::vec3 pos = {};
		glm::vec3 rotation = {};
		glm::vec3 scale = glm::vec3(1.0f);

		std::unique_ptr<Components::Component> components[(unsigned long long)COMPONENT_ID::COUNT];

		bool updateMatrices = true;
		glm::mat4 modelMatrix;
		glm::mat3 normalMatrix;
	};

	struct GameObjectsManager::Impl {
		Context* ctx;
		Scene* scene;

		uint32_t nextId = 1;

		std::unique_ptr<GameObject> rootGameObject;
		std::map<uint32_t, GameObject*> gameObjects;
	};
}
