#include <Yngin/Scenes.h>
#include <glad/glad.h>

namespace Yngin {
	Scene* Yngin::createScene(Context* ctx) {
		Scene* scene = nullptr;
		scene = new Scene(ctx);

		ctx->addScene(scene);

		return scene;
	}

	Scene::~Scene() {
	}

	void Scene::render() {
		ctx->makeCurrent();

		// TODO: render all objects
	}
}
