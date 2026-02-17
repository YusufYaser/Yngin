#include <Yngin/Scenes.h>
#include <Yngin/Models.h>
#include <Yngin/Cameras.h>
#include <glad/glad.h>
#include <stdexcept>
#include "Scenes_Internal.h"
#include <glm/gtc/type_ptr.hpp>

namespace Yngin {
	Scene::Scene(Context* ctx) {
		impl = std::make_unique<Impl>();
		impl->ctx = ctx;
		impl->camerasManager = std::make_unique<CamerasManager>(ctx, this);
	}

	Scene::~Scene() {
	}

	Context* Scene::getContext() {
		return impl->ctx;
	}

	CamerasManager* Scene::getCamerasManager() {
		return impl->camerasManager.get();
	}

	void Scene::render() {
		impl->ctx->makeCurrent();

		glClear(GL_COLOR_BUFFER_BIT);

		glm::mat4 proj = getCamerasManager()->getFinalProjection();
		glm::mat4 view = getCamerasManager()->getFinalView();

		GLuint shaderId = impl->ctx->getShaderId();
		glUseProgram(shaderId);
		GLuint projLoc = glGetUniformLocation(shaderId, "projection");
		GLuint viewLoc = glGetUniformLocation(shaderId, "view");

		glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(proj));
		glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));

		// for now we'll render a test model
		impl->ctx->getModelsManager()->render(0);
		// TODO: render all objects
	}

	ScenesManager::ScenesManager(Context* ctx) {
		if (ctx->getScenesManager()) {
			throw std::invalid_argument("Context already has a scene manager!");
		}

		this->ctx = ctx;
	}

	ScenesManager::~ScenesManager() = default;

	CamerasManager* ScenesManager::getCamerasManager(uint32_t sceneId) {
		auto it = scenes.find(sceneId);
		assert(it != scenes.end());
		if (it == scenes.end()) return nullptr;

		return it->second->getCamerasManager();
	}

	uint32_t ScenesManager::createScene() {
		uint32_t sceneId = nextSceneId++;
		scenes[sceneId] = std::make_unique<Scene>(ctx);
		return sceneId;
	}

	void ScenesManager::deleteScene(uint32_t sceneId) {
		assert(scenes.find(sceneId) != scenes.end());

		scenes.erase(sceneId);
	}

	void ScenesManager::render(uint32_t sceneId) {
		auto it = scenes.find(sceneId);
		assert(it != scenes.end());
		if (it == scenes.end()) return;

		it->second->render();
	}
}
