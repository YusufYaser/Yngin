#pragma once
#include "Components.h"
#include <memory>

namespace Yngin {
	namespace Components {
		class Mesh : public Component {
		public:
			void setModel(uint32_t newModelId);
			uint32_t getModel();

			void setTexture(uint32_t newTextureId);
			uint32_t getTexture();

		private:
			Mesh(Context* ctx, GameObject* gameObject);
			~Mesh();

			friend class GameObject;
			friend class std::default_delete<Mesh>;

			struct Impl;
			std::unique_ptr<Impl> impl;

			void onRender();
		};
	}
}
