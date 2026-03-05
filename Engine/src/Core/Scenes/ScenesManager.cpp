#include <Yngin/Core/Scenes.h>
#include <Yngin/Renderer/Cameras.h>
#include <stdexcept>
#include "Scenes_Internal.h"

namespace Yngin {
	ScenesManager::ScenesManager(Context* ctx) {
		if (ctx->getScenesManager()) {
			throw std::invalid_argument("Context already has a scene manager!");
		}

		impl = std::make_unique<Impl>();

		impl->ctx = ctx;
	}

	ScenesManager::~ScenesManager() = default;

	Scene* ScenesManager::getScene(uint32_t sceneId) {
		auto it = impl->scenes.find(sceneId);
		if (it == impl->scenes.end()) return nullptr;

		return it->second.get();
	}

	Scene* ScenesManager::createScene() {
		Scene* scene = new Scene(impl->ctx);

		uint32_t sceneId = impl->nextId++;
		scene->impl->id = sceneId;
		impl->scenes[sceneId] = std::unique_ptr<Scene>(scene);

		scene->impl->init();

		return scene;
	}

	void ScenesManager::deleteScene(uint32_t sceneId) {
		if (impl->activeScene->getId() == sceneId) {
			impl->activeScene = nullptr;
		}

		impl->scenes.erase(sceneId);
	}

	Scene* ScenesManager::getActive() {
		return impl->activeScene;
	}

	void ScenesManager::setActive(uint32_t sceneId) {
		impl->activeScene = getScene(sceneId);
	}
}
