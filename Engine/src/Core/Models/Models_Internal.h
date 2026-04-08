#pragma once
#include <Yngin/Core/Context.h>
#include <glad/glad.h>
#include <map>

namespace Yngin {
	struct Model::Impl {
		Context* ctx;
		void init(const ModelData& d);

		uint32_t id;

		GLuint VAO;
		GLuint VBO;
		GLuint EBO;

		GLsizei indicesCount;

		ModelData modelData;

		uint32_t materials[256];
	};

	struct ModelsManager::Impl {
		Context* ctx;

		std::map<uint32_t, std::unique_ptr<Model>> models;
		uint32_t nextId = 0;
	};
}
