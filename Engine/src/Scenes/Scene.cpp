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
}
