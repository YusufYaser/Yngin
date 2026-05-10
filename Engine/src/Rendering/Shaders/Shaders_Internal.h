#pragma once
#include <Yngin/Rendering/Shaders.h>
#include <Yngin/Core/Context.h>
#include <glad/glad.h>

namespace Yngin {
	struct Shader::Impl {
		SHADER_TYPE type;
		Context* ctx;

		GLuint glId;

		std::map<std::string, GLuint> uniformLocationsCache;
		GLuint getUniformLocationCached(const char* name);
	};

	struct ShadersManager::Impl {
		Context* ctx;
		Shader* activeShader = 0;

		uint32_t nextId = 0;

		std::map<SHADER_TYPE, std::unique_ptr<Shader>> shaders;
	};
}
