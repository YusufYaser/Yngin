#pragma once
#include <Yngin/Yngin.h>
#include <Yngin/Scenes.h>
#include <glm/vec3.hpp>

namespace Yngin {
	class GameObject {
	public:
		uint32_t getId();

		GameObject* getParent();
		void setParent(GameObject* newParent);

		GameObject* getChild(uint32_t childId);
		uint32_t createChild();
		void deleteChild(uint32_t childId);

		void moveChild(uint32_t childId, GameObject* newParent);

		glm::vec3 getPos();
		void setPos(glm::vec3 newPos);

		template <typename T>
		T* getComponent();

		template <typename T>
		void createComponent();

		template <typename T>
		void deleteComponent();

		// calls onRender() on all components and renders child OameObjects
		void render();

	private:
		friend class Scene;
		friend class GameObjectsManager;
		friend struct std::default_delete<GameObject>;

		GameObject(Context* ctx, Scene* scene, GameObject* parent);
		~GameObject();

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
