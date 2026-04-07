#include <Yngin/Core/Models.h>
#include <Yngin/Core/Scenes.h>
#include <Yngin/Rendering/Shaders.h>
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

	void Model::Impl::init(const ModelData& d) {
		if (d.vertices.size() == 0 || d.indices.size() == 0) {
			throw std::invalid_argument("Vertices and indices size cannot be zero");
		}

		ctx->makeCurrent();

		indicesCount = static_cast<GLsizei>(d.indices.size());

		glGenVertexArrays(1, &VAO);
		glBindVertexArray(VAO);

		glGenBuffers(1, &VBO);
		glBindBuffer(GL_ARRAY_BUFFER, VBO);
		glBufferData(GL_ARRAY_BUFFER, d.vertices.size() * sizeof(d.vertices[0]), d.vertices.data(), GL_STATIC_DRAW);

		glGenBuffers(1, &EBO);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, d.indices.size() * sizeof(d.indices[0]), d.indices.data(), GL_STATIC_DRAW);

		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, pos));
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, texCoord));
		glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, normal));
		glVertexAttribIPointer(3, 1, GL_UNSIGNED_INT, sizeof(Vertex), (void*)offsetof(Vertex, matId));
		glEnableVertexAttribArray(0);
		glEnableVertexAttribArray(1);
		glEnableVertexAttribArray(2);
		glEnableVertexAttribArray(3);

		glBindVertexArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

		this->frontFace = frontFace;

		this->modelData = d;

		if (this->modelData.materialsCount == 0) this->modelData.materialsCount = 1;
	}

	uint32_t Model::getId() const {
		return impl->id;
	}

	Context* Model::getContext() const {
		return impl->ctx;
	}

	void Model::render() {
		impl->ctx->makeCurrent();

		ModelsManager* mgr = impl->ctx->getModelsManager();

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

		Shader* shader = impl->ctx->getShadersManager()->getActive();

		Scene* scene = impl->ctx->getScenesManager()->getActive();

		glDrawElements(GL_TRIANGLES, impl->indicesCount, GL_UNSIGNED_INT, 0);
		glBindVertexArray(0);
	}

	const ModelData& Model::getModelData() const {
		return impl->modelData;
	}
}
