#include <Yngin/Scenes.h>
#include <Yngin/Models.h>
#include <glad/glad.h>
#include <stdexcept>

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
	}

	ScenesManager::~ScenesManager() {
		for (const auto& kvp : scenes) {
			delete kvp.second;
		}
		scenes.clear();
	}

	uint32_t ScenesManager::createScene() {
		Scene* scene = new Scene(ctx);
		uint32_t sceneId = nextSceneId++;
		scenes[sceneId] = scene;
		return sceneId;
	}

	void ScenesManager::renderScene(uint32_t sceneId) {
		if (!scenes.contains(sceneId)) {
			throw std::invalid_argument("Scene ID doesn't exist");
		}

		scenes[sceneId]->render();
	}
}
