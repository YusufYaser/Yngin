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

		uint64_t frame;
		double deltaTime;
		double lastFrameEnd;
		int maxFPS;

		// this will be removed later
		Model* skyboxModel;
	};
}
