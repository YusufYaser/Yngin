#include <Yngin/Scenes.h>
#include <Yngin/Cameras.h>
#include <stdexcept>
#include "Scenes_Internal.h"

namespace Yngin {
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
