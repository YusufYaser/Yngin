#pragma once
#include <Yngin/Core/Context.h>
#include <glad/glad.h>
#include <map>

namespace Yngin {
	struct Model::Impl {
		Context* ctx;
		void init(std::vector<Vertex> vertices, std::vector<uint32_t> indices);

		uint32_t id;

		GLuint VAO;
		GLuint VBO;
		GLuint EBO;

		GLsizei indicesCount;
	};

	struct ModelsManager::Impl {
		Context* ctx;

		std::map<uint32_t, std::unique_ptr<Model>> models;
		uint32_t nextId = 0;

		void loadObj(const char* data, size_t length, std::vector<Vertex>& vertices, std::vector<uint32_t>& indices);
	};
}
