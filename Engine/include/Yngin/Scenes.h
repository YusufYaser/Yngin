#pragma once
#include <Yngin/Yngin.h>
#include <memory>

namespace Yngin {
	class Scene;

	class ScenesManager {
	public:
		ScenesManager(Context* ctx);
		~ScenesManager();

		uint32_t createScene();
		void deleteScene(uint32_t sceneId);

		void renderScene(uint32_t sceneId);

	private:
		Context* ctx;

		std::map<uint32_t, std::unique_ptr<Scene>> scenes;
		uint32_t nextSceneId = 0;
	};
}
