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
#include <glm/gtc/type_ptr.hpp>

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

	uint32_t Scene::getId() {
		return impl->id;
	}

	Context* Scene::getContext() {
		return impl->ctx;
	}

	void Scene::activate() {
		impl->ctx->getScenesManager()->setActive(impl->id);
	}

	CamerasManager* Scene::getCamerasManager() {
		return impl->camerasManager.get();
	}

	GameObjectsManager* Scene::getGameObjectsManager() {
		return impl->gameObjectsManager.get();
	}

	UI::UIManager* Scene::getUIManager() {
		return impl->uiManager.get();
	}

	uint32_t Scene::getSkyboxTextureId() {
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

	void Scene::render() {
		impl->ctx->makeCurrent();

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glm::mat4 proj = getCamerasManager()->impl->getFinalPerspectiveProjection();
		glm::mat4 view = getCamerasManager()->impl->getFinalView();

		Model* skybox = impl->ctx->getSkyboxModel();
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

		impl->gameObjectsManager->getRootGameObject()->render();
		impl->uiManager->getRootElement()->render();
	}
}
