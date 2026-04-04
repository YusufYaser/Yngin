#include <Yngin/Core/Context.h>
#include <Yngin/UI/Elements/Button.h>
#include <Yngin/UI/Elements/Image.h>
#include <Yngin/UI/Elements/Text.h>
#include <Yngin/UI/Elements/UIElement.h>
#include <Yngin/Rendering/Shaders.h>
#include "UI_Elements_Internal.h"

namespace Yngin::UI {
	Button::Button(Context* ctx, Scene* scene, UIManager* mgr, UIElement* parent) : UIElement(ctx, scene, mgr, parent) {
		impl = std::make_unique<Impl>();

		UIElement::impl->size = { 0, 256, 0, 64 };

		impl->image = std::unique_ptr<Image>(new Image(ctx, scene, nullptr, this));
		impl->text = std::unique_ptr<Text>(new Text(ctx, scene, nullptr, this));

		impl->text->setTextSize(0);
		impl->text->setTextCentered({ 1, 1 });
		impl->text->setGlyph(mgr->getDefaultTextGlyph());
	}

	Button::~Button() = default;

	UI_TYPE Button::getType() const {
		return UI_TYPE::BUTTON;
	}

	Text* Button::getTextElement() {
		return impl->text.get();
	}

	Image* Button::getImage() {
		return impl->image.get();
	}

	glm::vec4 Button::getHoverColor() {
		return impl->hoverColor;
	}

	void Button::setHoverColor(glm::vec4 color) {
		impl->hoverColor = color;
	}

	glm::vec4 Button::getClickColor() {
		return impl->clickColor;
	}

	void Button::setClickColor(glm::vec4 color) {
		impl->clickColor = color;
	}

	void Button::render() {
		impl->image->setSize(UIElement::impl->size);
		impl->image->setPivot(UIElement::impl->pivot);
		impl->image->setPosition(UIElement::impl->pos);
		impl->image->setColor(UIElement::impl->color);

		impl->text->setSize(UIElement::impl->size);
		impl->text->setPosition({
			UIElement::impl->pos.xScale - UIElement::impl->size.xScale,
			UIElement::impl->pos.xOffset - UIElement::impl->size.xOffset,
			UIElement::impl->pos.yScale - UIElement::impl->size.yScale,
			UIElement::impl->pos.yOffset - UIElement::impl->size.yOffset,
			});

		glm::vec4 originalImageColor = impl->image->getColor();

		if (isHeld()) {
			impl->image->setColor(impl->clickColor);
		} else if (isHovered()) {
			impl->image->setColor(impl->hoverColor);
		}

		impl->image->render();
		impl->text->render();

		impl->image->setColor(originalImageColor);
	}
}
