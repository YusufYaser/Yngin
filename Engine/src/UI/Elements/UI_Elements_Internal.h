#pragma once
#include <Yngin/UI/UI.h>
#include <map>

namespace Yngin {
	namespace UI {
		struct UIElement::Impl {
			Context* ctx;
			Scene* scene;

			uint32_t id;

			UIElement* parent;

			std::map<uint32_t, std::unique_ptr<UIElement>> childs;

			UITransform pos = {};
			UITransform size = { 0.5f, 0, 0.5f, 0 };
		};

		struct Image::Impl {
			uint32_t texId;
		};
	}
}
