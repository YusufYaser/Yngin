#include <Yngin/Models.h>
#include <glad/glad.h>
#include <stdexcept>
#include "Models_Internal.h"

namespace Yngin {
	Model::Model(Context* ctx, std::vector<Vertex> vertices, std::vector<uint32_t> indices) {
		if (vertices.size() == 0 || indices.size() == 0) {
			throw std::invalid_argument("Vertices and indices size cannot be zero");
		}

		impl = std::make_unique<Impl>();

		ctx->makeCurrent();

		auto& m = *impl;
		m.ctx = ctx;

		m.indicesCount = indices.size();

		glGenVertexArrays(1, &m.VAO);
		glBindVertexArray(m.VAO);

		glGenBuffers(1, &m.VBO);
		glBindBuffer(GL_ARRAY_BUFFER, m.VBO);
		glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(vertices[0]), vertices.data(), GL_STATIC_DRAW);

		glGenBuffers(1, &m.EBO);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m.EBO);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(indices[0]), indices.data(), GL_STATIC_DRAW);

		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, pos));
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, texCoord));
		glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, normal));
		glEnableVertexAttribArray(0);
		glEnableVertexAttribArray(1);
		glEnableVertexAttribArray(2);

		glBindVertexArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	}

	Model::~Model() {
		impl->ctx->makeCurrent();

		glDeleteVertexArrays(1, &impl->VAO);
		glDeleteBuffers(1, &impl->VBO);
		glDeleteBuffers(1, &impl->VAO);
	}

	void Model::render() {
		impl->ctx->makeCurrent();

		glBindVertexArray(impl->VAO);
		glDrawElements(GL_TRIANGLES, impl->indicesCount, GL_UNSIGNED_INT, 0);
		glBindVertexArray(0);
	}
}
