#include <Yngin/Yngin.h>
#include <Yngin/Models.h>
#include <Yngin/Scenes.h>
#include <glad/glad.h>
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <stdexcept>
#include "Context_Internal.h"

// TODO: change this initial shader code
const char* vertexShaderCode = R"(
#version 460 core

layout(location = 0) in vec3 pos;

uniform mat4 projection;
uniform mat4 view;

void main() {
	gl_Position = projection * view * vec4(pos, 1.0);
}
)";

const char* fragmentShaderCode = R"(
#version 460 core

out vec4 FragColor;

void main() {
	FragColor = vec4(1.0f);
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

		m.glfwWindow = glfwCreateWindow(800, 600, "Yngin Game", nullptr, nullptr);
		makeCurrent();
		glfwSetFramebufferSizeCallback(m.glfwWindow, fb_resize_callback);
		gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);

		glViewport(0, 0, 800, 600);
		glClearColor(0, 0, 0, 1);
		glClear(GL_COLOR_BUFFER_BIT);

		glfwSwapBuffers(m.glfwWindow);

		m.modelsManager = std::make_unique<ModelsManager>(this);
		m.scenesManager = std::make_unique<ScenesManager>(this);

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

	uint32_t Context::getShaderId() {
		return impl->shader;
	}
}
