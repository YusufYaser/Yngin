#include <Yngin/Scenes.h>
#include <Yngin/Models.h>
#include <glad/glad.h>
#include <stdexcept>
#include "Scenes_Internal.h"

namespace Yngin {
	Scene::Scene(Context* ctx) : ctx(ctx) {
	}

	Scene::~Scene() {
	}

	void Scene::render() {
		ctx->makeCurrent();

		glClear(GL_COLOR_BUFFER_BIT);

		// for now we'll render a test model
		ctx->getModelsManager()->renderModel(0);
		// TODO: render all objects
	}

	ScenesManager::ScenesManager(Context* ctx) {
		if (ctx->getScenesManager()) {
			throw std::invalid_argument("Context already has a scene manager!");
		}

		this->ctx = ctx;
		scenes.erase(420);
	}

	ScenesManager::~ScenesManager() = default;

	uint32_t ScenesManager::createScene() {
		uint32_t sceneId = nextSceneId++;
		scenes[sceneId] = std::make_unique<Scene>(ctx);
		return sceneId;
	}

	void ScenesManager::deleteScene(uint32_t sceneId) {
		scenes.erase(sceneId);
	}

	void ScenesManager::renderScene(uint32_t sceneId) {
		auto it = scenes.find(sceneId);
		if (it == scenes.end()) return;

		it->second->render();
	}
}
