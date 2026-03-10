#pragma once
#include <memory>
#include <glm/vec3.hpp>

namespace Yngin {
	class CamerasManager;
	class GameObjectsManager;
	class GameObject;
	class Scene;
	class Context;
	class Texture;

	struct LightSettings {
		glm::vec3 ambientLight = glm::vec3(0.1f);
	};

	namespace UI {
		class UIManager;
	};

	class ScenesManager {
	public:
		Scene* getScene(uint32_t sceneId) const;

		Scene* createScene();
		void deleteScene(uint32_t sceneId);
		void deleteScene(Scene* scene);

		Scene* getActive() const;
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
		uint32_t getId() const;

		Context* getContext() const;

		void activate();

		CamerasManager* getCamerasManager() const;
		GameObjectsManager* getGameObjectsManager() const;
		UI::UIManager* getUIManager() const;

		uint32_t getSkyboxTextureId() const;
		void setSkyboxTexture(uint32_t texId);
		void setSkyboxTexture(Texture* tex);

		LightSettings getLightSettings() const;
		void setLightSettings(const LightSettings& lightSettings);

	private:
		friend class ScenesManager;
		friend struct std::default_delete<Scene>;
		friend class Context;

		void render();

		struct Impl;
		std::unique_ptr<Impl> impl;

		Scene(Context* ctx);
		~Scene();
	};
}
