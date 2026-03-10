#pragma once
#include <Yngin/UI/UI.h>
#include <map>

namespace Yngin {
	namespace UI {
		struct UIElement::Impl {
			Context* ctx;
			Scene* scene;
			UIManager* mgr;

			uint32_t id;

			UIElement* parent;

			std::map<uint32_t, std::unique_ptr<UIElement>> childs;

			glm::vec4 color = glm::vec4(1.0f);

			UITransform pos = { 0.5f, 0, 0.5f, 0 };
			UITransform size = { 0.5f, 0, 0.5f, 0 };
			UICrop crop = {};

			glm::vec2 pivot = glm::vec2(0.5f);

			void prepareUniforms();
		};

		struct Image::Impl {
			uint32_t texId;
		};

		struct Text::Impl {
			std::string text = "Text";
			int textSize = 24;
			uint32_t texId;
			glm::ivec2 spacing = {};
			glm::ivec2 centering = {};

			mutable glm::ivec2 textDims = { -1, -1 };
		};
	}
}
