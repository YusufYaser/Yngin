#pragma once
#include <glm/vec3.hpp>
#include <memory>
#include "../Services/Tween.h"

namespace Yngin {
	class Context;
	class Scene;

	namespace Physics {
		class PhysicsEngine;
	}

	class GameObject {
	public:
		uint32_t getId() const;
		Context* getContext() const;
		Scene* getScene() const;

		GameObject* getParent() const;
		void setParent(uint32_t newParentId);
		void setParent(GameObject* newParent);

		GameObject* getChild(uint32_t childId) const;
		GameObject* createChild();
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

		GameObject(Context* ctx, Scene* scene, GameObject* parent);
		~GameObject();

		// calls onRender() on all components and renders child OameObjects
		void render();

		struct Impl;
		std::unique_ptr<Impl> impl;
	};

	class GameObjectsManager {
	public:
		GameObject* getRootGameObject() const;
		GameObject* getGameObject(uint32_t gameObjectId) const;

		void deleteGameObject(uint32_t gameObjectId);
		void deleteGameObject(GameObject* gameObject);

	private:
		friend class Scene;
		friend struct std::default_delete<GameObjectsManager>;
		friend class GameObject;
		friend class Physics::PhysicsEngine;

		GameObjectsManager(Context* ctx, Scene* scene);
		~GameObjectsManager();

		uint32_t acquireId();

		struct Impl;
		std::unique_ptr<Impl> impl;
	};
}
