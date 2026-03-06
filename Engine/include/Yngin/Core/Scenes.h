#pragma once
#include <memory>

namespace Yngin {
	class CamerasManager;
	class GameObjectsManager;
	class GameObject;
	class Scene;
	class Context;
	class Texture;

	namespace UI {
		class UIManager;
	};

	class ScenesManager {
	public:
		Scene* getScene(uint32_t sceneId);

		Scene* createScene();
		void deleteScene(uint32_t sceneId);
		void deleteScene(Scene* scene);

		Scene* getActive();
		void setActive(uint32_t sceneId);
		void setActive(Scene* scene);

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

		void activate();

		CamerasManager* getCamerasManager();
		GameObjectsManager* getGameObjectsManager();
		UI::UIManager* getUIManager();

		uint32_t getSkyboxTextureId();
		void setSkyboxTexture(uint32_t texId);
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
