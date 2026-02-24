#include <Yngin/Components/Mesh.h>
#include "Components_Internal.h"
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glad/glad.h>
#include <Yngin/Textures.h>
#include <Yngin/Models.h>
#include <Yngin/GameObject.h>

namespace Yngin {
	namespace Components {
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

		Mesh::Mesh(Context* ctx, GameObject* gameObject) : Component(ctx, gameObject) {
			impl = std::make_unique<Impl>();
		}

		Mesh::~Mesh() = default;

		void Mesh::onRender() {
			auto cimpl = Component::impl.get();

			glm::mat4 modelMat = glm::mat4(1.0f);

			modelMat = glm::translate(modelMat, cimpl->gameObject->getPos());
			modelMat = glm::rotate(modelMat, glm::radians(90.0f), glm::vec3(1, 0, 0));

			GLuint shaderId = cimpl->ctx->getShaderId();
			glUseProgram(shaderId);
			GLuint modelLoc = glGetUniformLocation(shaderId, "model");
			GLuint normalMatrizLoc = glGetUniformLocation(shaderId, "normalMatrix");

			glm::mat3 normalMatrix = glm::transpose(glm::inverse(glm::mat3(modelMat)));

			glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(modelMat));
			glUniformMatrix3fv(normalMatrizLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));

			// for now we'll render a test model with a test texture
			cimpl->ctx->getTexturesManager()->getTexture(impl->texId)->activate();
			cimpl->ctx->getModelsManager()->render(impl->modelId);
		}
	}
}
