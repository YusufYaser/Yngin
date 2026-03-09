#pragma once
#include "UIElement.h"

namespace Yngin {
	class Texture;

	namespace UI {
		class Image : public UIElement {
		public:
			static const UI_TYPE staticType = UI_TYPE::IMAGE;

			UI_TYPE getType() const;

			void setTexture(uint32_t newTextureId);
			void setTexture(Texture* newTexture);
			uint32_t getTexture() const;

		private:
			friend class UIElement;
			friend struct std::default_delete<Image>;

			Image(Context* ctx, Scene* scene, UIManager* mgr, UIElement* parent);
			~Image();

			void render();

			struct Impl;
			std::unique_ptr<Impl> impl;
		};
	}
}
