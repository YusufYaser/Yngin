#pragma once
#include <Yngin/UI/UI.h>
#include <map>

namespace Yngin {
	namespace UI {
		struct UIManager::Impl {
			Context* ctx;
			Scene* scene;

			uint32_t nextId = 1;

			std::unique_ptr<UIElement> rootElement;

			std::map<uint32_t, UIElement*> elements;

			uint32_t defaultTextGlyph = 0;
		};
	}
}
