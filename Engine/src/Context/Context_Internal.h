#pragma once
#include <Yngin/Yngin.h>

namespace Yngin {
	struct Context::Impl {
		GLFWwindow* glfwWindow;

		std::unique_ptr<ModelsManager> modelsManager;
		std::unique_ptr<ScenesManager> scenesManager;
		std::unique_ptr<TexturesManager> texturesManager;

		// TODO: add a custom shader class
		uint32_t shader;
	};
}
