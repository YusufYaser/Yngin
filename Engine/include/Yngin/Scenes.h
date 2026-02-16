#pragma once
#include <Yngin/Yngin.h>
#include <memory>

namespace Yngin {
	class Scene;
	class CamerasManager;

	class ScenesManager {
	public:
		ScenesManager(Context* ctx);
		~ScenesManager();

		CamerasManager* getCamerasManager(uint32_t sceneId);

		uint32_t createScene();
		void deleteScene(uint32_t sceneId);

		void render(uint32_t sceneId);

	private:
		Context* ctx;

		std::map<uint32_t, std::unique_ptr<Scene>> scenes;
		uint32_t nextSceneId = 0;
	};
}
