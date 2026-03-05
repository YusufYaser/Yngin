#pragma once
#include "UIElement.h"

namespace Yngin {
	namespace UI {
		class Image : public UIElement {
		public:
			static const UIType staticType = UIType::Image;

			UIType getType();

			void setTexture(uint32_t newTextureId);
			uint32_t getTexture();

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
