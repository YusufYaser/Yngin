#pragma once
#include "UIElement.h"
#include <string>

namespace Yngin {
	class Texture;

	namespace UI {
		class Text : public UIElement {
		public:
			static const UIType staticType = UIType::Text;

			UIType getType();

			void setText(std::string newText);
			std::string getText();

			void setTextSize(int textSize);
			int getTextSize();
			void setSize(int textSize);
			int getSize();

			void setGlyph(Texture* texture);
			void setGlyph(uint32_t texId);
			uint32_t getGlyph();

			void setCrop(UICrop newCrop) override;
			UICrop getCrop() override;

			void setPivot(glm::vec2 newPivot) override;
			glm::vec2 getPivot() override;

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
