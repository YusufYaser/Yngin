#include <Yngin/Yngin.h>
#include <Yngin/Models.h>
#include <Yngin/Scenes.h>
#include <Yngin/Textures.h>
#include <glad/glad.h>
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <stdexcept>
#include "Context_Internal.h"

// TODO: change this initial shader code
const char* vertexShaderCode = R"(
#version 460 core

layout(location = 0) in vec3 inPos;
layout(location = 1) in vec2 inTexCoord;
layout(location = 2) in vec3 inNormal;

uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;
uniform mat3 normalMatrix;

out vec2 fTexCoord;
out vec3 fNormal;

void main() {
	gl_Position = projection * view * model * vec4(inPos, 1.0);

	fTexCoord = inTexCoord;
	fNormal = normalize(normalMatrix * inNormal);
}
)";

const char* fragmentShaderCode = R"(
#version 460 core

in vec2 fTexCoord;
in vec3 fNormal;

out vec4 FragColor;

uniform sampler2D tex0;

void main() {
	FragColor = vec4(fNormal, 1.0);
}
)";

void fb_resize_callback(GLFWwindow* window, int width, int height) {
	GLFWwindow* oldContext = glfwGetCurrentContext();
	glfwMakeContextCurrent(window);
	glViewport(0, 0, width, height);
	glfwMakeContextCurrent(oldContext);
}

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

		m.glfwWindow = glfwCreateWindow(800, 600, "Yngin Instance", nullptr, nullptr);
		makeCurrent();
		glfwSetFramebufferSizeCallback(m.glfwWindow, fb_resize_callback);
		gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);

		glViewport(0, 0, 800, 600);
		glClearColor(0, 0, 0, 1);
		glClear(GL_COLOR_BUFFER_BIT);

		glEnable(GL_DEPTH_TEST);
		glDepthFunc(GL_LESS);

		glfwSwapBuffers(m.glfwWindow);

		m.modelsManager = std::unique_ptr<ModelsManager>(new ModelsManager(this));
		m.scenesManager = std::unique_ptr<ScenesManager>(new ScenesManager(this));
		m.texturesManager = std::unique_ptr<TexturesManager>(new TexturesManager(this));

		// load initial shader
		// TODO: show error logs
		bool shadersFailed = false;
		GLuint vertexShader, fragmentShader;
		{
			vertexShader = glCreateShader(GL_VERTEX_SHADER);
			glShaderSource(vertexShader, 1, &vertexShaderCode, 0);
			glCompileShader(vertexShader);
			GLint shaderCompiled = 0;
			glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &shaderCompiled);
			if (!shaderCompiled) {
				shadersFailed = true;
			}
		}
		{
			fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
			glShaderSource(fragmentShader, 1, &fragmentShaderCode, 0);
			glCompileShader(fragmentShader);
			GLint shaderCompiled = 0;
			glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &shaderCompiled);
			if (!shaderCompiled) {
				shadersFailed = true;
			}
		}

		m.shader = glCreateProgram();
		glAttachShader(m.shader, vertexShader);
		glAttachShader(m.shader, fragmentShader);
		glLinkProgram(m.shader);
		glDeleteShader(vertexShader);
		glDeleteShader(fragmentShader);

		if (shadersFailed) {
			throw std::exception("Failed to initialize shaders");
		}

		glUseProgram(m.shader);
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
		glUseProgram(0);
		glDeleteProgram(m.shader);
		glfwDestroyWindow(m.glfwWindow);
	}

	void Yngin::Context::deleteAllContexts() {
		for (Context* ctx : contexts) {
			delete ctx;
		}
	}

	void Context::makeCurrent() {
		if (glfwGetCurrentContext() != impl->glfwWindow) {
			glfwMakeContextCurrent(impl->glfwWindow);
		}
	}

	void Context::updateWindow() {
		makeCurrent();

		glfwPollEvents();
	}

	bool Context::windowShouldClose() {
		return glfwWindowShouldClose(impl->glfwWindow);
	}

	void Context::swapBuffers() {
		glfwSwapBuffers(impl->glfwWindow);
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

	uint32_t Context::getShaderId() {
		return impl->shader;
	}
}
