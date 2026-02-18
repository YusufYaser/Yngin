#pragma once
#include <Yngin/Yngin.h>
#include <Yngin/Scenes.h>
#include <glm/vec3.hpp>

namespace Yngin {
	class GameObject {
	public:
		~GameObject();

		uint32_t getId();

		GameObject* getParent();
		void setParent(GameObject* newParent);

		GameObject* getChild(uint32_t childId);
		uint32_t createChild();
		void deleteChild(uint32_t childId);

		void moveChild(uint32_t childId, GameObject* newParent);

		glm::vec3 getPos();
		void setPos(glm::vec3 newPos);

		// this will be changed later
		void render();

	private:
		friend class Scene;
		friend class GameObjectsManager;

		GameObject(Context* ctx, Scene* scene, GameObject* parent);

		struct Impl;
		std::unique_ptr<Impl> impl;
	};

	class GameObjectsManager {
	public:
		GameObjectsManager(Context* ctx, Scene* scene);
		~GameObjectsManager();

		uint32_t acquireId();

		GameObject* getRootGameObject();
		GameObject* getGameObject(uint32_t gameObjectId);

		void deleteGameObject(uint32_t gameObjectId);

	private:
		friend class Scene;
		friend class GameObject;

		struct Impl;
		std::unique_ptr<Impl> impl;
	};
}
