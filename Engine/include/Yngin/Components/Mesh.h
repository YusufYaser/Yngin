#pragma once
#include <memory>
#include <glm/vec3.hpp>
#include "Component.h"

namespace Yngin {
	class Context;
	class GameObject;

	namespace Components {
		class Mesh : public Component {
		public:
			void setModel(uint32_t newModelId);
			uint32_t getModel();

			void setTexture(uint32_t newTextureId);
			uint32_t getTexture();

			void setScale(glm::vec3 newScale);
			glm::vec3 getScale();

		private:
			Mesh(Context* ctx, GameObject* gameObject);
			~Mesh();

			friend class GameObject;
			friend struct std::default_delete<Mesh>;

			struct Impl;
			std::unique_ptr<Impl> impl;

			void onRender();
		};
	}
}
