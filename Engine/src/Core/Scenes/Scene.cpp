#include <Yngin/Core/Scenes.h>
#include <Yngin/Core/Models.h>
#include <Yngin/UI/Elements/UIElement.h>
#include <Yngin/UI/UIManager.h>
#include <Yngin/Renderer/Shaders.h>
#include <Yngin/Renderer/Cameras.h>
#include <Yngin/Renderer/Textures.h>
#include <glad/glad.h>
#include <stdexcept>
#include "Scenes_Internal.h"
#include "../../Renderer/Cameras/Cameras_Internal.h"
#include "../../Core/GameObject/GameObject_Internal.h"
#include <glm/gtc/type_ptr.hpp>
#include <string>
#include "../../Core/Context/Context_Internal.h"

#define MAX_LIGHTS 32

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

	void Scene::render() {
		impl->ctx->makeCurrent();

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glm::mat4 proj = getCamerasManager()->impl->getFinalPerspectiveProjection();
		glm::mat4 view = getCamerasManager()->impl->getFinalView();

		Model* skybox = impl->ctx->getInternalModelsManager()->getModel(INTERNAL_MODEL_SKYBOX_ID);
		Texture* skyboxTex = impl->ctx->getTexturesManager()->getTexture(impl->skyboxTexId);
		if (skyboxTex && skybox) {
			Shader* skyboxShader = impl->ctx->getShadersManager()->getShader(SHADER_TYPE::SKYBOX);
			skyboxShader->activate();

			skyboxShader->setMat4("projection", proj);
			skyboxShader->setMat4("view", glm::mat4(glm::mat3(view)));

			skyboxTex->activate();
			skybox->render();
			glClear(GL_DEPTH_BUFFER_BIT);
		}

		Shader* worldShader = impl->ctx->getShadersManager()->getShader(SHADER_TYPE::WORLD);
		worldShader->activate();

		worldShader->setMat4("projection", proj);
		worldShader->setMat4("view", view);

		// register lights
		int lightsCount = 0;
		for (auto& kvp : impl->gameObjectsManager->impl->gameObjects) {
			GameObject* obj = kvp.second;
			Components::Light* light = obj->getComponent<Components::Light>();

			if (light == nullptr) continue;

			worldShader->setVec3(std::string("lights[" + std::to_string(lightsCount) + "].position").c_str(), obj->getPosition());
			worldShader->setVec3(std::string("lights[" + std::to_string(lightsCount) + "].color").c_str(), light->getColor());
			worldShader->setFloat(std::string("lights[" + std::to_string(lightsCount) + "].distance").c_str(), light->getDistance());
			worldShader->setFloat(std::string("lights[" + std::to_string(lightsCount) + "].intensity").c_str(), light->getIntensity());

			lightsCount++;

			if (lightsCount >= MAX_LIGHTS) {
				break;
			}
		}

		worldShader->setInt("lightsCount", lightsCount);

		worldShader->setVec3("cameraPos", impl->camerasManager->getFinalPos());

		worldShader->setVec3("scene.ambientLight", impl->lightSettings.ambientLight);

		impl->gameObjectsManager->getRootGameObject()->render();
		impl->uiManager->getRootElement()->render();
	}
}
