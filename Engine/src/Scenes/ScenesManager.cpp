#include <Yngin/Scenes.h>
#include <Yngin/Cameras.h>
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

	CamerasManager* ScenesManager::getCamerasManager(uint32_t sceneId) {
		auto it = impl->scenes.find(sceneId);
		assert(it != impl->scenes.end());
		if (it == impl->scenes.end()) return nullptr;

		return it->second->getCamerasManager();
	}

	uint32_t ScenesManager::createScene() {
		auto scene = std::unique_ptr<Scene>(new Scene(impl->ctx));

		uint32_t sceneId = impl->nextSceneId++;
		impl->scenes[sceneId] = std::move(scene);
		return sceneId;
	}

	void ScenesManager::deleteScene(uint32_t sceneId) {
		assert(impl->scenes.find(sceneId) != impl->scenes.end());

		impl->scenes.erase(sceneId);
	}

	void ScenesManager::render(uint32_t sceneId) {
		auto it = impl->scenes.find(sceneId);
		assert(it != impl->scenes.end());
		if (it == impl->scenes.end()) return;

		it->second->render();
	}
}
