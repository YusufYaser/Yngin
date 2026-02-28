#include <Yngin/Renderer/Shaders.h>
#include "Shaders_Internal.h"
#include <glm/mat4x4.hpp>
#include <glm/gtc/type_ptr.hpp>

namespace Yngin {
	Shader::Shader(Context* ctx, SHADER_TYPE type) {
		impl = std::make_unique<Impl>();

		impl->ctx = ctx;
		impl->type = type;
	}

	Shader::~Shader() = default;

	SHADER_TYPE Shader::getType() {
		return impl->type;
	}

	bool Shader::setSource(const ShaderSource& src) {
		bool shadersFailed = false;
		GLuint vertexShader, fragmentShader;
		{
			vertexShader = glCreateShader(GL_VERTEX_SHADER);
			glShaderSource(vertexShader, 1, &src.vertex, 0);
			glCompileShader(vertexShader);
			GLint shaderCompiled = 0;
			glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &shaderCompiled);
			if (!shaderCompiled) {
				shadersFailed = true;
			}
		}
		{
			fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
			glShaderSource(fragmentShader, 1, &src.fragment, 0);
			glCompileShader(fragmentShader);
			GLint shaderCompiled = 0;
			glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &shaderCompiled);
			if (!shaderCompiled) {
				shadersFailed = true;
			}
		}

		if (shadersFailed) return false;

		bool wasActive = impl->ctx->getShadersManager()->getActive() == this;

		if (wasActive) glUseProgram(0);

		glDeleteProgram(impl->glId);

		GLuint id = glCreateProgram();
		glAttachShader(id, vertexShader);
		glAttachShader(id, fragmentShader);
		glLinkProgram(id);
		glDeleteShader(vertexShader);
		glDeleteShader(fragmentShader);

		if (wasActive) glUseProgram(id);

		impl->glId = id;

		return !shadersFailed;
	}

	void Shader::activate() {
		impl->ctx->getShadersManager()->impl->activeShader = this;
		glUseProgram(impl->glId);
	}

	void Shader::setMat4(const char* name, glm::mat4 v) {
		GLuint loc = glGetUniformLocation(impl->glId, name);
		glUniformMatrix4fv(loc, 1, GL_FALSE, glm::value_ptr(v));
	}

	void Shader::setMat3(const char* name, glm::mat3 v) {
		GLuint loc = glGetUniformLocation(impl->glId, name);
		glUniformMatrix3fv(loc, 1, GL_FALSE, glm::value_ptr(v));
	}
}
