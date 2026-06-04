#pragma once
#include <Yngin/Forward.h>
#include <memory>
#include <glm/vec3.hpp>
#include <Yngin/Utils/Meta.h>

namespace Yngin {
	struct LightSettings {
		glm::vec3 ambientLight = glm::vec3(0.1f);
	};

	class ScenesManager {
	public:
		Scene* getScene(uint32_t sceneId) const;

		std::vector<Scene*> getScenes() const;
		size_t getScenesCount() const;

		Scene* createScene();
		Scene* createScene(const char* scenePakData, size_t size);
		Scene* createScene(uint32_t id, bool override = false);
		Scene* createScene(const char* scenePakData, size_t size, uint32_t id, bool override = false);
		void deleteScene(uint32_t sceneId);
		void deleteScene(Scene* scene);

		Scene* getActive() const;
		void setActive(uint32_t sceneId);
		void setActive(Scene* scene);

		static bool validateScenePak(const char* scenePakData, size_t size);

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

		Meta meta;

		void activate();

		CamerasManager* getCamerasManager() const;
		GameObjectsManager* getGameObjectsManager() const;
		UI::UIManager* getUIManager() const;

		uint32_t getSkyboxTextureId() const;
		void setSkyboxTexture(uint32_t texId);
		void setSkyboxTexture(Texture* tex);

		LightSettings getLightSettings() const;
		void setLightSettings(const LightSettings& lightSettings);

		float getGravity();
		void setGravity(float gravity);

		std::vector<char> generatePak();

	private:
		friend class ScenesManager;
		friend struct std::default_delete<Scene>;
		friend class Context;
		friend class Physics::PhysicsEngine;
		friend class Rendering::Renderer;
		friend class Components::Component;

		struct Impl;
		std::unique_ptr<Impl> impl;

		Scene(Context* ctx);
		~Scene();
	};
}
