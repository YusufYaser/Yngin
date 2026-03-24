#pragma once
#include <Yngin/Forward.h>
#include <memory>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#include "Component.h"

namespace Yngin {
	namespace Components {
		class Mesh : public Component {
		public:
			void setModel(uint32_t newModelId);
			void setModel(Model* newModel);
			uint32_t getModel() const;

			void setTexture(uint32_t newTextureId);
			void setTexture(Texture* newTexture);
			uint32_t getTexture() const;

			void setColor(glm::vec3 newColor);
			glm::vec3 getColor() const;

		private:
			Mesh(GameObject* gameObject);
			~Mesh();

			friend class GameObject;
			friend struct std::default_delete<Mesh>;
			friend class Rendering::Renderer;

			struct Impl;
			std::unique_ptr<Impl> impl;
		};
	}
}
