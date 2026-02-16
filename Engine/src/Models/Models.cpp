#include <Yngin/Models.h>
#include <glad/glad.h>
#include <stdexcept>
#include "Models_Internal.h"

namespace Yngin {
	struct Model::StructureInfo {
		GLsizei indicesCount;
	};

	struct Model::IDs {
		GLuint VAO;
		GLuint VBO;
		GLuint EBO;
	};

	Model::Model(Context* ctx, std::vector<Vertex> vertices, std::vector<uint32_t> indices) : ctx(ctx) {
		if (vertices.size() == 0 || indices.size() == 0) {
			throw std::invalid_argument("Vertices and indices size cannot be zero");
		}

		ctx->makeCurrent();

		structureInfo = std::make_unique<StructureInfo>();
		structureInfo->indicesCount = indices.size();

		ids = std::make_unique<IDs>();
		glGenVertexArrays(1, &ids->VAO);
		glBindVertexArray(ids->VAO);

		glGenBuffers(1, &ids->VBO);
		glBindBuffer(GL_ARRAY_BUFFER, ids->VBO);
		glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(vertices[0]), vertices.data(), GL_STATIC_DRAW);

		glGenBuffers(1, &ids->EBO);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ids->EBO);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(indices[0]), indices.data(), GL_STATIC_DRAW);

		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, pos));
		glEnableVertexAttribArray(0);

		glBindVertexArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	}

	Model::~Model() {
		ctx->makeCurrent();

		glDeleteVertexArrays(1, &ids->VAO);
		glDeleteBuffers(1, &ids->VBO);
		glDeleteBuffers(1, &ids->VAO);
	}

	void Model::render() {
		ctx->makeCurrent();

		glBindVertexArray(ids->VAO);
		glDrawElements(GL_TRIANGLES, structureInfo->indicesCount, GL_UNSIGNED_INT, 0);
		glBindVertexArray(0);
	}

	ModelsManager::ModelsManager(Context* ctx) {
		if (ctx->getModelsManager()) {
			throw std::invalid_argument("Context already has a model manager!");
		}

		this->ctx = ctx;
	}

	ModelsManager::~ModelsManager() = default;

	uint32_t ModelsManager::createModel(std::vector<Vertex> vertices, std::vector<uint32_t> indices) {
		uint32_t modelId = nextModelId++;
		models[modelId] = std::make_unique<Model>(ctx, vertices, indices);
		return modelId;
	}

	void ModelsManager::deleteModel(uint32_t modelId) {
		models.erase(modelId);
	}

	void ModelsManager::renderModel(uint32_t modelId) {
		auto it = models.find(modelId);
		if (it == models.end()) return;

		it->second->render();
	}
}
