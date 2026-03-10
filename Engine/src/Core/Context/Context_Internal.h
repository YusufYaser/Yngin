#pragma once
#include <Yngin/Core/Context.h>
#include <Yngin/Services/Services.h>
#include <typeindex>

namespace Yngin {
	struct Context::Impl {
		CONTEXT_STATUS status = CONTEXT_STATUS::INITIALIZING;

		std::unique_ptr<Window> window;

		std::unique_ptr<ModelsManager> modelsManager;
		std::unique_ptr<ScenesManager> scenesManager;
		std::unique_ptr<TexturesManager> texturesManager;
		std::unique_ptr<ShadersManager> shadersManager;
		std::unique_ptr<UI::UIManager> uiManager;

		std::unique_ptr<Physics::PhysicsEngine> physicsEngine;
		std::unique_ptr<InputSystem> inputSystem;

		uint64_t frame = 0;
		double deltaTime = 1;
		double lastFrameEnd = 0;
		int maxFPS = -1;

		// these will be removed later
		Model* skyboxModel = nullptr;
		Model* imageModel = nullptr;

		std::map<std::type_index, std::unique_ptr<Services::Service>> services;
	};
}
