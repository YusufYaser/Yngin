#pragma once
#include <Yngin/Core/Context.h>

namespace Yngin {
	struct Context::Impl {
		std::unique_ptr<Window> window;

		std::unique_ptr<ModelsManager> modelsManager;
		std::unique_ptr<ScenesManager> scenesManager;
		std::unique_ptr<TexturesManager> texturesManager;
		std::unique_ptr<ShadersManager> shadersManager;

		uint64_t frame;
		double deltaTime;
		double lastFrameEnd;
		int maxFPS;
	};
}
