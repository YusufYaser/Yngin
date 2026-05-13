#include <Yngin/Core/Models.h>
#include <Yngin/Core/Scenes.h>
#include <Yngin/Rendering/Shaders.h>
#include <glad/glad.h>
#include <stdexcept>
#include "Models_Internal.h"
#include <Yngin/Core/Materials.h>

namespace Yngin {
	Model::Model(Context* ctx) {
		impl = std::make_unique<Impl>();
		impl->ctx = ctx;
		impl->owner = this;
	}

	Model::~Model() {
		impl->ctx->makeCurrent();

		impl->submeshes.clear();
	}

	InternalSubmesh::~InternalSubmesh() {
		glDeleteVertexArrays(1, &VAO);
		glDeleteBuffers(1, &VBO);
		glDeleteBuffers(1, &VAO);
	}

	void Model::Impl::init(const ModelData& d) {
		if (d.vertices.size() == 0 || d.indices.size() == 0) {
			throw std::invalid_argument("Vertices and indices size cannot be zero");
		}

		ctx->makeCurrent();

		submeshes.clear();

		indicesCount = static_cast<GLsizei>(d.indices.size());

		std::map<uint32_t, InternalSubmesh*> matSubmeshes;
		std::map<InternalSubmesh*, std::map<uint32_t, uint32_t>> indicesMap;
		std::map<InternalSubmesh*, std::vector<Vertex>> submeshesVertices;
		std::map<InternalSubmesh*, std::vector<uint32_t>> submeshesIndices;

		for (auto& index : d.indices) {
			if (d.vertices.size() <= index) continue;

			const Vertex& v = d.vertices[index];

			InternalSubmesh* submesh = nullptr;

			if (matSubmeshes.find(v.matId) == matSubmeshes.end()) {
				submesh = new InternalSubmesh();
				submesh->model = owner;
				submesh->matId = v.matId;
				matSubmeshes[v.matId] = submesh;
				submeshes.push_back(std::unique_ptr<InternalSubmesh>(submesh));
			} else {
				submesh = matSubmeshes[v.matId];
			}

			if (indicesMap[submesh].find(index) == indicesMap[submesh].end()) {
				uint32_t newIndex = submeshesVertices[submesh].size();
				submeshesVertices[submesh].push_back(v);
				indicesMap[submesh][index] = newIndex;
			}

			submeshesIndices[submesh].push_back(indicesMap[submesh][index]);
		}

		for (auto& submesh : submeshes) {
			auto& vertices = submeshesVertices[submesh.get()];
			auto& indices = submeshesIndices[submesh.get()];

			glGenVertexArrays(1, &submesh->VAO);
			glBindVertexArray(submesh->VAO);

			glGenBuffers(1, &submesh->VBO);
			glBindBuffer(GL_ARRAY_BUFFER, submesh->VBO);
			glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(vertices[0]), vertices.data(), GL_STATIC_DRAW);

			glGenBuffers(1, &submesh->EBO);
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, submesh->EBO);
			glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(indices[0]), indices.data(), GL_STATIC_DRAW);

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

			submesh->verticesCount = vertices.size();
			submesh->indicesCount = indices.size();

			glm::vec3 maxPos = glm::vec3(std::numeric_limits<float>::min());

			for (auto& v : vertices) {
				submesh->center += v.pos;
				maxPos = glm::max(maxPos, v.pos);
			}

			submesh->center /= vertices.size();
			submesh->radius = glm::distance(submesh->center, maxPos);
		}


		this->modelData = d;

		if (this->modelData.materialsCount == 0) this->modelData.materialsCount = 1;
	}

	uint16_t Model::getId() const {
		return impl->id;
	}

	Context* Model::getContext() const {
		return impl->ctx;
	}

	void Model::Impl::render(int instances) {
		ctx->makeCurrent();

		if (modelData.frontFace == MODEL_FRONT_FACE::NONE) {
			glDisable(GL_CULL_FACE);
		} else {
			glEnable(GL_CULL_FACE);
			if (modelData.frontFace == MODEL_FRONT_FACE::CW) {
				glFrontFace(GL_CW);
			} else {
				glFrontFace(GL_CCW);
			}
		}

		Shader* worldShader = ctx->getShadersManager()->getShader(SHADER_TYPE::WORLD);

		for (auto& submesh : submeshes) {
			glBindVertexArray(submesh->VAO);

			glDrawElementsInstanced(GL_TRIANGLES, submesh->indicesCount, GL_UNSIGNED_INT, 0, instances);
			glBindVertexArray(0);
		}
	}

	void Model::Impl::renderWithMaterials(const uint32_t materialsMap[256], int instances) {
		ctx->makeCurrent();

		if (modelData.frontFace == MODEL_FRONT_FACE::NONE) {
			glDisable(GL_CULL_FACE);
		} else {
			glEnable(GL_CULL_FACE);
			if (modelData.frontFace == MODEL_FRONT_FACE::CW) {
				glFrontFace(GL_CW);
			} else {
				glFrontFace(GL_CCW);
			}
		}

		Shader* worldShader = ctx->getShadersManager()->getShader(SHADER_TYPE::WORLD);

		for (auto& submesh : submeshes) {
			if (submesh->matId >= 256) continue;
			uint32_t matId = materialsMap[submesh->matId];
			Material* mat = ctx->getMaterialsManager()->getMaterial(matId);
			if (mat == nullptr) continue;

			worldShader->setVec3("material.ambientColor", mat->getAmbientColor());
			worldShader->setVec3("material.diffuseColor", mat->getDiffuseColor());
			worldShader->setVec3("material.specularColor", mat->getSpecularColor());
			worldShader->setFloat("material.specularComponent", mat->getSpecularComponent());

			glBindVertexArray(submesh->VAO);

			glDrawElementsInstanced(GL_TRIANGLES, submesh->indicesCount, GL_UNSIGNED_INT, 0, instances);
			glBindVertexArray(0);
		}
	}

	const ModelData& Model::getModelData() const {
		return impl->modelData;
	}

	size_t Model::getSubmeshesCount() const {
		return impl->submeshes.size();
	}
}
