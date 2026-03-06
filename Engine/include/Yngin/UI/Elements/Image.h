#pragma once
#include "UIElement.h"
#include <glm/vec4.hpp>

namespace Yngin {
	class Texture;

	namespace UI {
		class Image : public UIElement {
		public:
			static const UIType staticType = UIType::Image;

			UIType getType();

			void setTexture(uint32_t newTextureId);
			void setTexture(Texture* newTexture);
			uint32_t getTexture();

			void setColor(glm::vec4 newColor);
			glm::vec4 getColor();

		private:
			friend class UIElement;
			friend struct std::default_delete<Image>;

			Image(Context* ctx, Scene* scene, UIElement* parent);
			~Image();

			void render();

			struct Impl;
			std::unique_ptr<Impl> impl;
		};
	}
}
