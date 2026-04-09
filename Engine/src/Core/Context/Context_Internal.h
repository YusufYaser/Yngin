#pragma once
#include <Yngin/Core/Context.h>
#include <Yngin/Services/Services.h>
#include <typeindex>

#define INTERNAL_MODEL_SKYBOX_ID	0
#define INTERNAL_MODEL_SQUARE_ID	1

namespace Yngin {
	struct Context::Impl {
		CONTEXT_STATUS status = CONTEXT_STATUS::INITIALIZING;

		ContextSettings initialSettings;
		void applySettings(const ContextSettings& settings);

		std::unique_ptr<Window> window;

		std::unique_ptr<ModelsManager> modelsManager;
		std::unique_ptr<ScenesManager> scenesManager;
		std::unique_ptr<TexturesManager> texturesManager;
		std::unique_ptr<ShadersManager> shadersManager;
		std::unique_ptr<UI::UIManager> uiManager;
		std::unique_ptr<ScriptsManager> scriptsManager;
		std::unique_ptr<MaterialsManager> materialsManager;

		std::unique_ptr<Rendering::Renderer> renderer;
		std::unique_ptr<Physics::PhysicsEngine> physicsEngine;
		std::unique_ptr<InputSystem> inputSystem;

		std::unique_ptr<ModelsManager> internalModelsManager;

		glm::ivec4 forcedViewport;

		uint64_t frame = 0;
		double deltaTime = 1;
		double lastFrameEnd = 0;
		int maxFPS = -1;

		std::map<std::type_index, std::unique_ptr<Services::Service>> services;

		std::vector<PakLoadSettings> pakLoadSettings;
		std::vector<PakGenSettings> pakGenSettings;
	};
}
