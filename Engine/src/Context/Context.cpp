#include <Yngin/Yngin.h>
#include <Yngin/Models.h>
#include <Yngin/Scenes.h>
#include <glad/glad.h>
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <stdexcept>

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

		glfwWindow = glfwCreateWindow(800, 600, "Yngin Game", nullptr, nullptr);
		makeCurrent();
		glfwSetFramebufferSizeCallback(glfwWindow, fb_resize_callback);
		gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);

		glViewport(0, 0, 800, 600);
		glClearColor(0, 0, 0, 1);
		glClear(GL_COLOR_BUFFER_BIT);

		glfwSwapBuffers(glfwWindow);

		modelsManager = std::make_unique<ModelsManager>(this);
		scenesManager = std::make_unique<ScenesManager>(this);

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

		shader = glCreateProgram();
		glAttachShader(shader, vertexShader);
		glAttachShader(shader, fragmentShader);
		glLinkProgram(shader);
		glDeleteShader(vertexShader);
		glDeleteShader(fragmentShader);

		if (shadersFailed) {
			throw std::exception("Failed to initialize shaders");
		}

		glUseProgram(shader);
	}

	Context::~Context() {
		cleanup();
	}

	void Context::cleanup() {
		makeCurrent();

		modelsManager.reset();
		scenesManager.reset();

		contexts.erase(std::find(contexts.begin(), contexts.end(), this));
		glUseProgram(0);
		glDeleteProgram(shader);
		glfwDestroyWindow(glfwWindow);
	}

	void Yngin::Context::deleteAllContexts() {
		for (Context* ctx : contexts) {
			delete ctx;
		}
	}

	void Context::makeCurrent() {
		if (glfwGetCurrentContext() != glfwWindow) {
			glfwMakeContextCurrent(glfwWindow);
		}
	}

	void Context::updateWindow() {
		makeCurrent();

		glfwPollEvents();
	}

	bool Context::windowShouldClose() {
		return glfwWindowShouldClose(glfwWindow);
	}

	void Context::swapBuffers() {
		glfwSwapBuffers(glfwWindow);
	}

	glm::ivec2 Context::getViewportSize() {
		GLint viewportData[4];
		glGetIntegerv(GL_VIEWPORT, viewportData);
		return { viewportData[2], viewportData[3] };
	}

	ModelsManager* Context::getModelsManager() {
		return modelsManager.get();
	}

	ScenesManager* Context::getScenesManager() {
		return scenesManager.get();
	}

	uint32_t Context::getShaderId() {
		return shader;
	}
}
