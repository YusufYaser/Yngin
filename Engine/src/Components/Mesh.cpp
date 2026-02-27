#include <Yngin/Components/Mesh.h>
#include "Components_Internal.h"
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glad/glad.h>
#include <Yngin/Renderer/Textures.h>
#include <Yngin/Core/Models.h>
#include <Yngin/Core/GameObject.h>
#include <Yngin/Core/Context.h>
#include <Yngin/Renderer/Shaders.h>

namespace Yngin {
	namespace Components {
		Mesh::Mesh(Context* ctx, GameObject* gameObject) : Component(ctx, gameObject) {
			impl = std::make_unique<Impl>();
		}

		Mesh::~Mesh() = default;

		void Mesh::setModel(uint32_t newModelId) {
			impl->modelId = newModelId;
		}

		uint32_t Mesh::getModel() {
			return impl->modelId;
		}

		void Mesh::setTexture(uint32_t newTextureId) {
			impl->texId = newTextureId;
		}

		uint32_t Mesh::getTexture() {
			return impl->texId;
		}

		void Mesh::setScale(glm::vec3 newScale) {
			impl->scale = newScale;
		}

		glm::vec3 Mesh::getScale() {
			return impl->scale;
		}

		void Mesh::onRender() {
			auto cimpl = Component::impl.get();

			Model* model = cimpl->ctx->getModelsManager()->getModel(impl->modelId);
			if (model == nullptr) return;

			Texture* tex = cimpl->ctx->getTexturesManager()->getTexture(impl->texId);

			GameObject* obj = cimpl->gameObject;

			glm::mat4 modelMat = glm::mat4(1.0f);

			modelMat = glm::translate(modelMat, obj->getPos());
			modelMat = glm::rotate(modelMat, obj->getRotation().x, glm::vec3(1, 0, 0));
			modelMat = glm::rotate(modelMat, obj->getRotation().y, glm::vec3(0, 1, 0));
			modelMat = glm::rotate(modelMat, obj->getRotation().z, glm::vec3(0, 0, 1));
			modelMat = glm::scale(modelMat, impl->scale);

			Shader* worldShader = cimpl->ctx->getShadersManager()->getShader(SHADER_TYPE::WORLD);
			worldShader->activate();

			glm::mat3 normalMatrix = glm::transpose(glm::inverse(glm::mat3(modelMat)));

			worldShader->setMat3("normalMatrix", normalMatrix);
			worldShader->setMat4("model", modelMat);

			if (tex) tex->activate();
			model->render();
		}
	}
}
