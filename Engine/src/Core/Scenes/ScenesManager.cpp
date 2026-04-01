#include <Yngin/Core/Scenes.h>
#include <Yngin/Rendering/Cameras.h>
#include <stdexcept>
#include "Scenes_Internal.h"
#include <Yngin/Core/Scripting.h>
#include "../Scripting/Scripting_Internal.h"

namespace Yngin {
	ScenesManager::ScenesManager(Context* ctx) {
		if (ctx->getScenesManager()) {
			throw std::invalid_argument("Context already has a scene manager!");
		}

		impl = std::make_unique<Impl>();

		impl->ctx = ctx;
	}

	ScenesManager::~ScenesManager() = default;

	Scene* ScenesManager::getScene(uint32_t sceneId) const {
		auto it = impl->scenes.find(sceneId);
		if (it == impl->scenes.end()) return nullptr;

		return it->second.get();
	}

	std::vector<Scene*> ScenesManager::getScenes() const {
		std::vector<Scene*> scenes;
		for (auto& kvp : impl->scenes) {
			scenes.push_back(kvp.second.get());
		}
		return scenes;
	}

	size_t ScenesManager::getScenesCount() const {
		return impl->scenes.size();
	}

	Scene* ScenesManager::createScene() {
		return createScene(impl->nextId);
	}

	Scene* ScenesManager::createScene(const char* scenePakData, size_t size) {
		return createScene(scenePakData, size, impl->nextId);
	}

	Scene* ScenesManager::createScene(uint32_t id, bool override) {
		if (getScene(id) != nullptr) {
			if (override) {
				deleteScene(id);
			} else {
				return nullptr;
			}
		}

		Scene* scene = new Scene(impl->ctx);

		impl->nextId = std::max(impl->nextId, id + 1);
		scene->impl->id = id;
		impl->scenes[id] = std::unique_ptr<Scene>(scene);

		scene->impl->init();

		return scene;
	}

	Scene* ScenesManager::createScene(const char* scenePakData, size_t size, uint32_t id, bool override) {
		Scene* scene = createScene(id, override);

		scene->impl->loadPak(scenePakData, size);

		return scene;
	}

	void ScenesManager::deleteScene(uint32_t sceneId) {
		if (impl->activeScene && impl->activeScene->getId() == sceneId) {
			impl->activeScene = nullptr;
		}

		impl->scenes.erase(sceneId);
	}

	void ScenesManager::deleteScene(Scene* scene) {
		if (scene->impl->ctx == impl->ctx) {
			deleteScene(scene->impl->id);
		}
	}

	Scene* ScenesManager::getActive() const {
		return impl->activeScene;
	}

	void ScenesManager::setActive(uint32_t sceneId) {
		Scene* scene = getScene(sceneId);
		if (scene == impl->activeScene) return;

		impl->ctx->getScriptsManager()->impl->onSceneInactive();
		impl->activeScene = scene;
		impl->ctx->getScriptsManager()->impl->onSceneActive();
	}

	void ScenesManager::setActive(Scene* scene) {
		if (scene->impl->ctx == impl->ctx) {
			setActive(scene->impl->id);
		}
	}
}
