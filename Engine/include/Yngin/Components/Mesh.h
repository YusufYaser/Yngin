#pragma once
#include <memory>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#include "Component.h"

namespace Yngin {
	class Context;
	class GameObject;
	class Model;
	class Texture;

	namespace Components {
		class Mesh : public Component {
		public:
			void setModel(uint32_t newModelId);
			void setModel(Model* newModel);
			uint32_t getModel();

			void setTexture(uint32_t newTextureId);
			void setTexture(Texture* newTexture);
			uint32_t getTexture();

			void setColor(glm::vec3 newColor);
			glm::vec3 getColor();

			void setScale(glm::vec3 newScale);
			glm::vec3 getScale();

		private:
			Mesh(GameObject* gameObject);
			~Mesh();

			friend class GameObject;
			friend struct std::default_delete<Mesh>;

			struct Impl;
			std::unique_ptr<Impl> impl;

			void onRender();
		};
	}
}
