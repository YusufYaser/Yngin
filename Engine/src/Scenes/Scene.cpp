#include <Yngin/Core/Scenes.h>
#include <Yngin/Core/Models.h>
#include <Yngin/Renderer/Cameras.h>
#include <glad/glad.h>
#include <stdexcept>
#include "Scenes_Internal.h"
#include <glm/gtc/type_ptr.hpp>

namespace Yngin {
	Scene::Scene(Context* ctx) {
		impl = std::make_unique<Impl>();

		impl->ctx = ctx;
		impl->owner = this;
	}

	Scene::~Scene() = default;

	void Scene::Impl::init() {
		camerasManager = std::unique_ptr<CamerasManager>(new CamerasManager(ctx, owner));
		gameObjectsManager = std::unique_ptr<GameObjectsManager>(new GameObjectsManager(ctx, owner));
	}

	uint32_t Scene::getId() {
		return impl->id;
	}

	Context* Scene::getContext() {
		return impl->ctx;
	}

	CamerasManager* Scene::getCamerasManager() {
		return impl->camerasManager.get();
	}

	GameObjectsManager* Scene::getGameObjectsManager() {
		return impl->gameObjectsManager.get();
	}

	void Scene::render() {
		impl->ctx->makeCurrent();

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glm::mat4 proj = getCamerasManager()->getFinalProjection();
		glm::mat4 view = getCamerasManager()->getFinalView();

		GLuint shaderId = impl->ctx->getShaderId();
		glUseProgram(shaderId);
		GLuint projLoc = glGetUniformLocation(shaderId, "projection");
		GLuint viewLoc = glGetUniformLocation(shaderId, "view");

		glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(proj));
		glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));

		impl->gameObjectsManager->getRootGameObject()->render();
	}
}
