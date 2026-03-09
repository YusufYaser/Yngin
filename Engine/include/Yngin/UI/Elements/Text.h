#pragma once
#include "UIElement.h"
#include <string>

namespace Yngin {
	class Texture;

	namespace UI {
		class Text : public UIElement {
		public:
			static const UI_TYPE staticType = UI_TYPE::TEXT;

			UI_TYPE getType();

			void setText(std::string newText);
			std::string getText();

			// 0 for auto size based on the UI element size
			void setTextSize(int textSize);
			int getTextSize();

			void setGlyph(Texture* texture);
			void setGlyph(uint32_t texId);
			uint32_t getGlyph();

			void setSpacing(glm::ivec2 spacing);
			glm::ivec2 getSpacing();

			glm::ivec2 getTextDimensions();
			glm::ivec2 getTextDimensionsPixels();

			void setCrop(UICrop newCrop) override;
			UICrop getCrop() override;

		private:
			friend class UIElement;
			friend struct std::default_delete<Text>;

			Text(Context* ctx, Scene* scene, UIManager* mgr, UIElement* parent);
			~Text();

			void render();

			struct Impl;
			std::unique_ptr<Impl> impl;
		};
	}
}
