#pragma once
#include <Yngin/Core/Context.h>

namespace Yngin {
	struct Context::Impl {
		CONTEXT_STATUS status = CONTEXT_STATUS::INITIALIZING;

		std::unique_ptr<Window> window;

		std::unique_ptr<ModelsManager> modelsManager;
		std::unique_ptr<ScenesManager> scenesManager;
		std::unique_ptr<TexturesManager> texturesManager;
		std::unique_ptr<ShadersManager> shadersManager;

		uint64_t frame = 0;
		double deltaTime = 1;
		double lastFrameEnd = 0;
		int maxFPS = -1;

		// this will be removed later
		Model* skyboxModel = nullptr;
	};
}
