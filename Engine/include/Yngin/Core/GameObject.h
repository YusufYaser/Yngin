#pragma once
#include <glm/vec3.hpp>
#include <memory>
#include "../Services/Tween.h"

namespace Yngin {
	class Context;
	class Scene;

	class GameObject {
	public:
		uint32_t getId();
		Context* getContext();
		Scene* getScene();

		GameObject* getParent();
		void setParent(GameObject* newParent);

		GameObject* getChild(uint32_t childId);
		GameObject* createChild();
		void deleteChild(uint32_t childId);

		void moveChild(uint32_t childId, GameObject* newParent);

		glm::vec3 getPos();
		void setPos(glm::vec3 newPos);

		glm::vec3 getRotation();
		void setRotation(glm::vec3 newRotation);

		template <typename T>
		T* getComponent();

		template <typename T>
		T* createComponent();

		template <typename T>
		void deleteComponent();

	private:
		friend class Scene;
		friend class GameObjectsManager;
		friend class Services::Tween;
		friend struct std::default_delete<GameObject>;

		GameObject(Context* ctx, Scene* scene, GameObject* parent);
		~GameObject();

		// calls onRender() on all components and renders child OameObjects
		void render();

		struct Impl;
		std::unique_ptr<Impl> impl;
	};

	class GameObjectsManager {
	public:
		GameObject* getRootGameObject();
		GameObject* getGameObject(uint32_t gameObjectId);

		void deleteGameObject(uint32_t gameObjectId);

	private:
		friend class Scene;
		friend struct std::default_delete<GameObjectsManager>;
		friend class GameObject;

		GameObjectsManager(Context* ctx, Scene* scene);
		~GameObjectsManager();

		uint32_t acquireId();

		struct Impl;
		std::unique_ptr<Impl> impl;
	};
}
