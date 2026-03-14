#include <Yngin/Core/Scenes.h>
#include <Yngin/UI/UIManager.h>
#include <Yngin/Rendering/Cameras.h>
#include <Yngin/Rendering/Textures.h>
#include "Scenes_Internal.h"

namespace Yngin {
	Scene::Scene(Context* ctx) {
		impl = std::make_unique<Impl>();

		impl->ctx = ctx;
		impl->owner = this;
	}

	Scene::~Scene() = default;

	void Scene::Impl::init() {
		camerasManager = std::unique_ptr<CamerasManager>(new CamerasManager(owner));
		gameObjectsManager = std::unique_ptr<GameObjectsManager>(new GameObjectsManager(ctx, owner));
		uiManager = std::unique_ptr<UI::UIManager>(new UI::UIManager(ctx, owner));
	}

	uint32_t Scene::getId() const {
		return impl->id;
	}

	Context* Scene::getContext() const {
		return impl->ctx;
	}

	void Scene::activate() {
		impl->ctx->getScenesManager()->setActive(impl->id);
	}

	CamerasManager* Scene::getCamerasManager() const {
		return impl->camerasManager.get();
	}

	GameObjectsManager* Scene::getGameObjectsManager() const {
		return impl->gameObjectsManager.get();
	}

	UI::UIManager* Scene::getUIManager() const {
		return impl->uiManager.get();
	}

	uint32_t Scene::getSkyboxTextureId() const {
		return impl->skyboxTexId;
	}

	void Scene::setSkyboxTexture(uint32_t texId) {
		impl->skyboxTexId = texId;
	}

	void Scene::setSkyboxTexture(Texture* tex) {
		if (tex->getContext() == impl->ctx) {
			impl->skyboxTexId = tex->getId();
		}
	}

	LightSettings Scene::getLightSettings() const {
		return impl->lightSettings;
	}

	void Scene::setLightSettings(const LightSettings& lightSettings) {
		impl->lightSettings = lightSettings;
	}

	float Scene::getGravity() {
		return impl->gravity;
	}

	void Scene::setGravity(float gravity) {
		impl->gravity = gravity;
	}
}
