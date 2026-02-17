#pragma once
#include <Yngin/Yngin.h>

namespace Yngin {
	class CamerasManager;

	struct Scene::Impl {
		Context* ctx;
		std::unique_ptr<CamerasManager> camerasManager;
	};

	struct ScenesManager::Impl {
		Context* ctx;

		std::map<uint32_t, std::unique_ptr<Scene>> scenes;
		uint32_t nextSceneId = 0;
	};
}