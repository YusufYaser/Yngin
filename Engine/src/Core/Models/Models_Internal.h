#pragma once
#include <Yngin/Core/Models.h>
#include <Yngin/Core/Context.h>
#include <glad/glad.h>
#include <map>

namespace Yngin {
	struct InternalSubmesh {
		~InternalSubmesh();

		Model* model;

		GLuint VAO;
		GLuint VBO;
		GLuint EBO;
		uint32_t matId;

		size_t verticesCount;
		size_t indicesCount;
	};

	struct Model::Impl {
		Context* ctx;
		void init(const ModelData& d);

		Model* owner;

		void render(int instances = 1);
		void renderWithMaterials(const uint32_t materialsMap[256], int instances = 1);

		uint32_t id;

		std::vector<std::unique_ptr<InternalSubmesh>> submeshes;

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
