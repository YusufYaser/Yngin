#include <Yngin/Core/Models.h>
#include <glad/glad.h>
#include <stdexcept>
#include "Models_Internal.h"

namespace Yngin {
	Model::Model(Context* ctx) {
		impl = std::make_unique<Impl>();
		impl->ctx = ctx;
	}

	Model::~Model() {
		impl->ctx->makeCurrent();

		glDeleteVertexArrays(1, &impl->VAO);
		glDeleteBuffers(1, &impl->VBO);
		glDeleteBuffers(1, &impl->VAO);
	}

	void Model::Impl::init(std::vector<Vertex> vertices, std::vector<uint32_t> indices, MODEL_FRONT_FACE frontFace) {
		if (vertices.size() == 0 || indices.size() == 0) {
			throw std::invalid_argument("Vertices and indices size cannot be zero");
		}

		ctx->makeCurrent();

		indicesCount = static_cast<GLsizei>(indices.size());

		glGenVertexArrays(1, &VAO);
		glBindVertexArray(VAO);

		glGenBuffers(1, &VBO);
		glBindBuffer(GL_ARRAY_BUFFER, VBO);
		glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(vertices[0]), vertices.data(), GL_STATIC_DRAW);

		glGenBuffers(1, &EBO);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
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

		this->frontFace = frontFace;
	}

	uint32_t Model::getId() const {
		return impl->id;
	}

	Context* Model::getContext() const {
		return impl->ctx;
	}

	void Model::render() {
		impl->ctx->makeCurrent();

		if (impl->frontFace == MODEL_FRONT_FACE::NONE) {
			glDisable(GL_CULL_FACE);
		} else {
			glEnable(GL_CULL_FACE);
			if (impl->frontFace == MODEL_FRONT_FACE::CW) {
				glFrontFace(GL_CW);
			} else {
				glFrontFace(GL_CCW);
			}
		}

		glBindVertexArray(impl->VAO);
		glDrawElements(GL_TRIANGLES, impl->indicesCount, GL_UNSIGNED_INT, 0);
		glBindVertexArray(0);
	}
}
