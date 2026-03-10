#pragma once
#include "UIElement.h"
#include <string>

namespace Yngin {
	class Texture;

	namespace UI {
		class Button : public UIElement {
		public:
			static const UI_TYPE staticType = UI_TYPE::BUTTON;

			UI_TYPE getType() const;

			Text* getTextElement();
			Image* getImage();

			glm::ivec4 getHoverColor();
			void setHoverColor(glm::ivec4 color);

			glm::ivec4 getClickColor();
			void setClickColor(glm::ivec4 color);

		private:
			friend class UIElement;
			friend struct std::default_delete<Button>;

			Button(Context* ctx, Scene* scene, UIManager* mgr, UIElement* parent);
			~Button();

			void render();

			struct Impl;
			std::unique_ptr<Impl> impl;
		};
	}
}
