#include <Yngin/Components/Mesh.h>
#include "Components_Internal.h"
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glad/glad.h>
#include <Yngin/Rendering/Textures.h>
#include <Yngin/Core/Models.h>
#include <Yngin/Core/GameObject.h>
#include <Yngin/Core/Context.h>
#include <Yngin/Rendering/Shaders.h>

namespace Yngin {
	namespace Components {
		Mesh::Mesh(GameObject* gameObject) : Component(gameObject) {
			impl = std::make_unique<Impl>();
		}

		Mesh::~Mesh() = default;

		void Mesh::setModel(uint32_t newModelId) {
			impl->modelId = newModelId;
		}

		void Mesh::setModel(Model* newModel) {
			if (newModel->getContext() == Component::impl->ctx) {
				impl->modelId = newModel->getId();
			}
		}

		uint32_t Mesh::getModel() const {
			return impl->modelId;
		}

		void Mesh::setTexture(uint32_t newTextureId) {
			impl->texId = newTextureId;
		}

		void Mesh::setTexture(Texture* newTexture) {
			if (newTexture == nullptr) {
				setTexture(uint32_t(0));
				return;
			}
			if (newTexture->getContext() == Component::impl->ctx) {
				impl->texId = newTexture->getId();
			}
		}

		uint32_t Mesh::getTexture() const {
			return impl->texId;
		}

		void Mesh::setColor(glm::vec3 newColor) {
			impl->color = newColor;
		}

		glm::vec3 Mesh::getColor() const {
			return impl->color;
		}
	}
}
