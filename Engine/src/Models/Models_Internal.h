#pragma once
#include <Yngin/Yngin.h>

namespace Yngin {
	struct Model::Impl {
		Context* ctx;

		GLuint VAO;
		GLuint VBO;
		GLuint EBO;

		GLsizei indicesCount;
	};

	struct ModelsManager::Impl {
		Context* ctx;

		std::map<uint32_t, std::unique_ptr<Model>> models;
		uint32_t nextModelId = 0;
	};
}
