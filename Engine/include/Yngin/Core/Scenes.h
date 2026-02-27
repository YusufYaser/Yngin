#pragma once
#include <memory>

namespace Yngin {
	class CamerasManager;
	class GameObjectsManager;
	class GameObject;
	class Scene;
	class Context;
	class Texture;

	class ScenesManager {
	public:
		Scene* getScene(uint32_t sceneId);

		uint32_t createScene();
		void deleteScene(uint32_t sceneId);

		Scene* getActive();
		void setActive(uint32_t sceneId);

	private:
		friend class Context;
		friend struct std::default_delete<ScenesManager>;
		friend class Scene;

		ScenesManager(Context* ctx);
		~ScenesManager();

		struct Impl;
		std::unique_ptr<Impl> impl;
	};

	class Scene {
	public:
		uint32_t getId();

		Context* getContext();

		CamerasManager* getCamerasManager();
		GameObjectsManager* getGameObjectsManager();

		Texture* getSkyboxTexture();
		void setSkyboxTexture(Texture* tex);

		void render();

	private:
		friend class ScenesManager;
		friend struct std::default_delete<Scene>;

		struct Impl;
		std::unique_ptr<Impl> impl;

		Scene(Context* ctx);
		~Scene();
	};
}
