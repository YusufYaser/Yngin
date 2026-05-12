#include <Yngin/Rendering/Shaders.h>
#include "Shaders_Internal.h"
#include <glm/mat4x4.hpp>
#include <glm/gtc/type_ptr.hpp>

#define LOGGER_NAME Shaders
#include "../../Internal/Logger.h"

namespace Yngin {
	Shader::Shader(Context* ctx, SHADER_TYPE type) {
		impl = std::make_unique<Impl>();

		impl->ctx = ctx;
		impl->type = type;
	}

	Shader::~Shader() = default;

	SHADER_TYPE Shader::getType() const {
		return impl->type;
	}

	Context* Shader::getContext() const {
		return impl->ctx;
	}

	bool Shader::setSource(const ShaderSource& src, bool hasFragment) {
		DEBUG("Updating shader source for shader %i", getType());

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
				DEBUG("Failed to compile vertex shader %i", getType());

				GLint logLength;
				glGetShaderiv(vertexShader, GL_INFO_LOG_LENGTH, &logLength);

				if (logLength > 0) {
					std::vector<char> infoLog(logLength);
					glGetShaderInfoLog(vertexShader, logLength, NULL, &infoLog[0]);
					DEBUG("%s", &infoLog[0]);
				}
			}
		}
		if (hasFragment) {
			fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
			glShaderSource(fragmentShader, 1, &src.fragment, 0);
			glCompileShader(fragmentShader);
			GLint shaderCompiled = 0;
			glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &shaderCompiled);
			if (!shaderCompiled) {
				shadersFailed = true;
				DEBUG("Failed to compile fragment shader %i", getType());

				GLint logLength;
				glGetShaderiv(fragmentShader, GL_INFO_LOG_LENGTH, &logLength);

				if (logLength > 0) {
					std::vector<char> infoLog(logLength);
					glGetShaderInfoLog(fragmentShader, logLength, NULL, &infoLog[0]);
					DEBUG("%s", &infoLog[0]);
				}
			}
		}

		if (shadersFailed) return false;

		bool wasActive = impl->ctx->getShadersManager()->getActive() == this;

		if (wasActive) glUseProgram(0);

		glDeleteProgram(impl->glId);

		GLuint id = glCreateProgram();
		glAttachShader(id, vertexShader);
		if (hasFragment) glAttachShader(id, fragmentShader);
		glLinkProgram(id);
		glDeleteShader(vertexShader);
		if (hasFragment) glDeleteShader(fragmentShader);

		if (wasActive) glUseProgram(id);
		impl->uniformLocationsCache.clear();

		impl->glId = id;

		return !shadersFailed;
	}

	void Shader::activate() {
		impl->ctx->getShadersManager()->impl->activeShader = this;
		glUseProgram(impl->glId);
	}

	GLuint Shader::Impl::getUniformLocationCached(const char* name) {
		GLuint& cached = uniformLocationsCache[name];
		if (cached == 0) cached = glGetUniformLocation(glId, name);

		return cached;
	}

	void Shader::setMat4(const char* name, glm::mat4 v) {
		GLuint loc = impl->getUniformLocationCached(name);
		glUniformMatrix4fv(loc, 1, GL_FALSE, glm::value_ptr(v));
	}

	void Shader::setMat3(const char* name, glm::mat3 v) {
		GLuint loc = impl->getUniformLocationCached(name);
		glUniformMatrix3fv(loc, 1, GL_FALSE, glm::value_ptr(v));
	}

	void Shader::setFloat(const char* name, float v) {
		GLuint loc = impl->getUniformLocationCached(name);
		glUniform1f(loc, v);
	}

	void Shader::setInt(const char* name, int v) {
		GLuint loc = impl->getUniformLocationCached(name);
		glUniform1i(loc, v);
	}

	void Shader::setIVec2(const char* name, glm::ivec2 v) {
		GLuint loc = impl->getUniformLocationCached(name);
		glUniform2iv(loc, 1, glm::value_ptr(v));
	}

	void Shader::setVec2(const char* name, glm::vec2 v) {
		GLuint loc = impl->getUniformLocationCached(name);
		glUniform2fv(loc, 1, glm::value_ptr(v));
	}

	void Shader::setVec3(const char* name, glm::vec3 v) {
		GLuint loc = impl->getUniformLocationCached(name);
		glUniform3fv(loc, 1, glm::value_ptr(v));
	}

	void Shader::setVec4(const char* name, glm::vec4 v) {
		GLuint loc = impl->getUniformLocationCached(name);
		glUniform4fv(loc, 1, glm::value_ptr(v));
	}
}
