#include <Yngin/Yngin.h>
#include "../Window/Window_Internal.h"
#include <glad/glad.h>
#include <stdexcept>
#include "Context_Internal.h"
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <thread>
#include "../../Rendering/Shaders/Sources/World_Shader_Source.h"
#include "../../Rendering/Shaders/Sources/Skybox_Shader_Source.h"
#include "../../Rendering/Shaders/Sources/UI_Shader_Source.h"
#include "../../Physics/Physics_Internal.h"
#include "../../Rendering/Renderer/Renderer_Internal.h"
#include "../Models/Models_Internal.h"
#include "../Models/DefaultModels/Skybox_Model.h"
#include "../Models/DefaultModels/Square_Model.h"
#include <Yngin/Services/Services.h>

namespace Yngin {
	std::vector<Context*> Context::contexts;

	Context* createContext(const ContextSettings& settings) {
		Context* ctx = nullptr;
		if (isYnginInitialized()) ctx = new Context(settings);
		return ctx;
	}

	Context::Context(const ContextSettings& settings) {
		if (!isYnginInitialized()) {
			impl->status = CONTEXT_STATUS::FAILED_TO_INIT;
			throw std::exception("Cannot create new context before initialization");
		}
		contexts.push_back(this);

		impl = std::make_unique<Impl>();

		auto& m = *impl;

		m.window = std::unique_ptr<Window>(new Window(this, settings.windowSettings));

		glClearColor(0, 0, 0, 1);
		glClear(GL_COLOR_BUFFER_BIT);

		glEnable(GL_DEPTH_TEST);
		glDepthFunc(GL_LESS);

		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		m.modelsManager = std::unique_ptr<ModelsManager>(new ModelsManager(this));
		m.scenesManager = std::unique_ptr<ScenesManager>(new ScenesManager(this));
		m.texturesManager = std::unique_ptr<TexturesManager>(new TexturesManager(this));
		m.shadersManager = std::unique_ptr<ShadersManager>(new ShadersManager(this));
		m.uiManager = std::unique_ptr<UI::UIManager>(new UI::UIManager(this, nullptr));

		m.renderer = std::unique_ptr<Rendering::Renderer>(new Rendering::Renderer(this));
		m.physicsEngine = std::unique_ptr<Physics::PhysicsEngine>(new Physics::PhysicsEngine(this));
		m.inputSystem = std::unique_ptr<InputSystem>(new InputSystem(this));

		m.internalModelsManager = std::unique_ptr<ModelsManager>(new ModelsManager(this));

		m.services[std::type_index(typeid(Services::Tween))] = std::unique_ptr<Services::Tween>(new Services::Tween(this));

		TextureData texData{};
		texData.width = 1;
		texData.height = 1;
		texData.numCh = 1;
		texData.bytes = "\x00";
		m.texturesManager->createTexture(texData);

		Shader* worldShader = m.shadersManager->getShader(SHADER_TYPE::WORLD);
		Shader* skyboxShader = m.shadersManager->getShader(SHADER_TYPE::SKYBOX);
		Shader* uiShader = m.shadersManager->getShader(SHADER_TYPE::UI);
		bool shadersBuilt = worldShader->setSource(ShaderSources::world);
		shadersBuilt = shadersBuilt && skyboxShader->setSource(ShaderSources::skybox);
		shadersBuilt = shadersBuilt && uiShader->setSource(ShaderSources::ui);

		if (!shadersBuilt) {
			impl->status = CONTEXT_STATUS::FAILED_TO_INIT;
			throw std::exception("Failed to initialize shaders");
		}

		worldShader->activate();

		Model* skyboxModel = m.internalModelsManager->createModel(DefaultModels::skybox);
		{
			int oldId = skyboxModel->impl->id;
			int newId = INTERNAL_MODEL_SKYBOX_ID;

			skyboxModel->impl->id = newId;
			auto modelPtr = std::move(m.internalModelsManager->impl->models[oldId]);
			m.internalModelsManager->impl->models.erase(oldId);
			m.internalModelsManager->impl->models[newId] = std::move(modelPtr);
		}

		Model* imageModel = m.internalModelsManager->createModel(DefaultModels::square);
		{
			int oldId = imageModel->impl->id;
			int newId = INTERNAL_MODEL_SQUARE_ID;

			imageModel->impl->id = newId;
			auto modelPtr = std::move(m.internalModelsManager->impl->models[oldId]);
			m.internalModelsManager->impl->models.erase(oldId);
			m.internalModelsManager->impl->models[newId] = std::move(modelPtr);
		}

		impl->status = CONTEXT_STATUS::RUNNING;
	}

	Context::~Context() {
		cleanup();
	}

	void Context::cleanup() {
		auto& m = *impl;

		m.status = CONTEXT_STATUS::CLEANING_UP;

		makeCurrent();

		contexts.erase(std::find(contexts.begin(), contexts.end(), this));
	}

	ModelsManager* Context::getInternalModelsManager() const {
		return impl->internalModelsManager.get();
	}

	void Yngin::Context::deleteAllContexts() {
		for (Context* ctx : contexts) {
			delete ctx;
		}
	}

	void Context::makeCurrent() {
		impl->window->impl->makeCurrent();
	}

	CONTEXT_STATUS Context::getStatus() const {
		return impl->status;
	}

	void Context::update() {
		assert(getStatus() == CONTEXT_STATUS::RUNNING);
		makeCurrent();

		auto& m = *impl;

		glm::ivec2 windowSize = m.window->getSize();

		if (m.forcedViewport != glm::ivec4(0)) {
			glViewport(
				m.forcedViewport[0],
				windowSize.y - m.forcedViewport[1] - m.forcedViewport[3],
				m.forcedViewport[2],
				m.forcedViewport[3]
			);
		} else {
			glViewport(0, 0, windowSize.x, windowSize.y);
		}

		for (auto& kvp : m.services) {
			kvp.second.get()->onUpdate();
		}

		m.window->impl->update();
		m.inputSystem->onUpdate();

		Scene* scene = m.scenesManager->getActive();
		if (scene) {
			m.physicsEngine->impl->updatePhysics(scene);
			m.renderer->impl->render(scene);
		} else {
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		}

		m.renderer->impl->render(m.uiManager->getRootElement(), -1);

		glfwSwapInterval(m.maxFPS == 0);

		m.window->impl->swapBuffers();

		glViewport(0, 0, windowSize.x, windowSize.y);

		if (getStatus() == CONTEXT_STATUS::RUNNING && m.window->impl->shouldClose()) {
			m.status = CONTEXT_STATUS::NEEDS_TO_STOP;
		}

		m.frame++;

		if (m.maxFPS > 0) {
			double timeToFinish = m.lastFrameEnd + 1.0 / m.maxFPS;

			while (getTime() < timeToFinish) {
				if (timeToFinish - getTime() > 0.002) {
					std::this_thread::sleep_for(std::chrono::milliseconds(1));
				}
			}
		}

		double frameEnd = getTime();
		m.deltaTime = frameEnd - m.lastFrameEnd;
		m.lastFrameEnd = frameEnd;
	}

	uint64_t Context::getFrame() const {
		return impl->frame;
	}

	int Context::getMaxFPS() const {
		return impl->maxFPS;
	}

	void Context::setMaxFPS(int newMaxFPS) {
		impl->maxFPS = newMaxFPS;
	}

	double Context::getFrameStartTime() const {
		return impl->lastFrameEnd;
	}

	double Context::getTime() {
		makeCurrent();
		return glfwGetTime();
	}

	double Context::getDeltaTime() const {
		return impl->deltaTime;
	}

	Window* Context::getWindow() const {
		return impl->window.get();
	}

	void Context::forceViewport(glm::ivec2 pos, glm::ivec2 size) {
		impl->forcedViewport = glm::ivec4(pos, size);
	}

	glm::ivec4 Context::getForcedViewport() const {
		return impl->forcedViewport;
	}

	glm::ivec2 Context::getViewportPos() const {
		glm::ivec2 windowSize = impl->window->getSize();

		GLint viewportData[4];
		glGetIntegerv(GL_VIEWPORT, viewportData);
		return { viewportData[0], windowSize.y - viewportData[1] - viewportData[3] };
	}

	glm::ivec2 Context::getViewportSize() const {
		GLint viewportData[4];
		glGetIntegerv(GL_VIEWPORT, viewportData);
		return { viewportData[2], viewportData[3] };
	}

	ModelsManager* Context::getModelsManager() const {
		return impl->modelsManager.get();
	}

	ScenesManager* Context::getScenesManager() const {
		return impl->scenesManager.get();
	}

	TexturesManager* Context::getTexturesManager() const {
		return impl->texturesManager.get();
	}

	ShadersManager* Context::getShadersManager() const {
		return impl->shadersManager.get();
	}

	UI::UIManager* Context::getGlobalUIManager() const {
		return impl->uiManager.get();
	}

	Rendering::Renderer* Context::getRenderer() const {
		return impl->renderer.get();
	}

	Physics::PhysicsEngine* Context::getPhysicsEngine() const {
		return impl->physicsEngine.get();
	}

	InputSystem* Context::getInputSystem() const {
		return impl->inputSystem.get();
	}



	template<typename T>
	inline T* Context::getService() const {
		return dynamic_cast<T*>(impl->services[std::type_index(typeid(T))].get());
	}

	template Services::Tween* Context::getService<Services::Tween>() const;
}
