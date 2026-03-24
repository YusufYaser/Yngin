#pragma once
#include <Yngin/Forward.h>
#include "UIElement.h"
#include <string>

namespace Yngin {
	namespace UI {
		class Button : public UIElement {
		public:
			static const UI_TYPE staticType = UI_TYPE::BUTTON;

			UI_TYPE getType() const;

			Text* getTextElement();
			Image* getImage();

			glm::vec4 getHoverColor();
			void setHoverColor(glm::vec4 color);

			glm::vec4 getClickColor();
			void setClickColor(glm::vec4 color);

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
