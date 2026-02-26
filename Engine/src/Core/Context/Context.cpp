#include <Yngin/Core/Models.h>
#include <Yngin/Core/Scenes.h>
#include <Yngin/Renderer/Shaders.h>
#include <Yngin/Renderer/Textures.h>
#include <Yngin/Core/Window.h>
#include "../Window/Window_Internal.h"
#include <glad/glad.h>
#include <stdexcept>
#include "Context_Internal.h"
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <thread>
#include "../../Renderer/Shaders/Shader_Sources.h"

namespace Yngin {
	std::vector<Context*> Context::contexts;

	Context* createContext() {
		Context* ctx = nullptr;
		if (Yngin::isInitialized()) ctx = new Context();
		return ctx;
	}

	Context::Context() {
		if (!Yngin::isInitialized()) {
			throw std::exception("Cannot create new context before initialization");
		}
		contexts.push_back(this);

		impl = std::make_unique<Impl>();

		auto& m = *impl;

		m.deltaTime = 1;
		m.lastFrameEnd = 0;
		m.maxFPS = -1;

		m.window = std::unique_ptr<Window>(new Window(this));

		glClearColor(0, 0, 0, 1);
		glClear(GL_COLOR_BUFFER_BIT);

		glEnable(GL_DEPTH_TEST);
		glDepthFunc(GL_LESS);

		m.modelsManager = std::unique_ptr<ModelsManager>(new ModelsManager(this));
		m.scenesManager = std::unique_ptr<ScenesManager>(new ScenesManager(this));
		m.texturesManager = std::unique_ptr<TexturesManager>(new TexturesManager(this));
		m.shadersManager = std::unique_ptr<ShadersManager>(new ShadersManager(this));

		m.texturesManager->createTexture(1, 1, 1, "\x00");

		Shader* worldShader = m.shadersManager->getShader(SHADER_TYPE::WORLD);
		bool shadersBuilt = worldShader->setSource(ShaderSources::world);

		if (!shadersBuilt) {
			throw std::exception("Failed to initialize shaders");
		}

		worldShader->activate();
	}

	Context::~Context() {
		cleanup();
	}

	void Context::cleanup() {
		auto& m = *impl;

		makeCurrent();

		m.modelsManager.reset();
		m.scenesManager.reset();
		m.texturesManager.reset();

		contexts.erase(std::find(contexts.begin(), contexts.end(), this));
	}

	void Yngin::Context::deleteAllContexts() {
		for (Context* ctx : contexts) {
			delete ctx;
		}
	}

	void Context::makeCurrent() {
		impl->window->impl->makeCurrent();
	}

	bool Context::isClosing() {
		return impl->window->impl->shouldClose();
	}

	void Context::update() {
		assert(!isClosing());

		auto& m = *impl;

		m.window->impl->update();

		Scene* scene = m.scenesManager->getActive();
		assert(scene);
		if (scene) {
			m.scenesManager->getActive()->render();
		}

		glfwSwapInterval(m.maxFPS == 0);

		m.window->impl->swapBuffers();

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

	uint64_t Context::getFrame() {
		return impl->frame;
	}

	int Context::getMaxFPS() {
		return impl->maxFPS;
	}

	void Context::setMaxFPS(int newMaxFPS) {
		impl->maxFPS = newMaxFPS;
	}

	double Context::getTime() {
		makeCurrent();
		return glfwGetTime();
	}

	double Context::getDeltaTime() {
		return impl->deltaTime;
	}

	Window* Context::getWindow() {
		return impl->window.get();
	}

	glm::ivec2 Context::getViewportSize() {
		GLint viewportData[4];
		glGetIntegerv(GL_VIEWPORT, viewportData);
		return { viewportData[2], viewportData[3] };
	}

	ModelsManager* Context::getModelsManager() {
		return impl->modelsManager.get();
	}

	ScenesManager* Context::getScenesManager() {
		return impl->scenesManager.get();
	}

	TexturesManager* Context::getTexturesManager() {
		return impl->texturesManager.get();
	}

	ShadersManager* Context::getShadersManager() {
		return impl->shadersManager.get();
	}
}
