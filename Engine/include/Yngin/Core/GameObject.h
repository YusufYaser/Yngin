#pragma once
#include <Yngin/Forward.h>
#include <glm/vec3.hpp>
#include <memory>
#include "../Services/Tween.h"
#include <string>
#include <Yngin/Utils/Meta.h>

namespace Yngin {
	namespace GameFiles {
		class Generators;
	}

	class GameObject {
	public:
		uint32_t getId() const;
		Context* getContext() const;
		Scene* getScene() const;

		Meta meta;

		GameObject* getParent() const;
		void setParent(uint32_t newParentId);
		void setParent(GameObject* newParent);

		std::vector<GameObject*> getChildren() const;

		GameObject* getChild(uint32_t childId) const;
		GameObject* createChild();
		GameObject* createChild(uint32_t customId, bool override = true);
		void deleteChild(uint32_t childId);
		void deleteChild(GameObject* child);

		void moveChild(uint32_t childId, GameObject* newParent);
		void moveChild(GameObject* child, GameObject* newParent);
		void moveChild(GameObject* child, uint32_t newParentId);
		void moveChild(uint32_t childId, uint32_t newParentId);

		glm::vec3 getPosition() const;
		void setPosition(glm::vec3 newPos);

		glm::vec3 getRotation() const;
		void setRotation(glm::vec3 newRotation);

		glm::vec3 getScale() const;
		void setScale(glm::vec3 scale);

		template <typename T>
		bool hasComponent() const;

		template <typename T>
		T* getComponent() const;

		template <typename T>
		T* createComponent();

		template <typename T>
		void deleteComponent();

	private:
		friend class Scene;
		friend class GameObjectsManager;
		friend class Services::Tween;
		friend struct std::default_delete<GameObject>;
		friend class Physics::PhysicsEngine;
		friend class Rendering::Renderer;
		friend class GameFiles::Generators;
		friend class Components::Component;

		GameObject(Context* ctx, Scene* scene, GameObject* parent);
		~GameObject();

		struct Impl;
		std::unique_ptr<Impl> impl;
	};

	class GameObjectsManager {
	public:
		Context* getContext() const;

		GameObject* getRootGameObject() const;
		GameObject* getGameObject(uint32_t gameObjectId) const;
		GameObject* createGameObject();
		GameObject* createGameObject(uint32_t customId, bool override = true);

		size_t getGameObjectsCount() const;
		std::vector<GameObject*> getGameObjects() const;

		void deleteGameObject(uint32_t gameObjectId);
		void deleteGameObject(GameObject* gameObject);

	private:
		friend class Scene;
		friend struct std::default_delete<GameObjectsManager>;
		friend class GameObject;
		friend class Physics::PhysicsEngine;
		friend class Rendering::Renderer;
		friend class Components::Component;

		GameObjectsManager(Context* ctx, Scene* scene);
		~GameObjectsManager();

		struct Impl;
		std::unique_ptr<Impl> impl;
	};
}
