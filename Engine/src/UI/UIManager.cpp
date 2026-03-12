#include <Yngin/UI/UIManager.h>
#include "UI_Internal.h"
#include "Elements/UI_Elements_Internal.h"
#include <assert.h>
#include <Yngin/Rendering/Textures.h>

namespace Yngin::UI {
	UIManager::UIManager(Context* ctx, Scene* scene) {
		impl = std::make_unique<Impl>();
		impl->ctx = ctx;
		impl->scene = scene;

		impl->rootElement = std::unique_ptr<UIElement>(new UIElement(ctx, scene, this, nullptr));
		impl->rootElement->setSize({ 1.0f, 0, 1.0f, 0 });
		impl->rootElement->setPivot({ 0, 0 });
		impl->rootElement->setPosition({ 0, 0, 0, 0 });

		impl->elements[impl->rootElement->getId()] = impl->rootElement.get();
	}

	UIManager::~UIManager() = default;

	uint32_t UIManager::acquireId() {
		return impl->nextId++;
	}

	UIElement* UIManager::getRootElement() const {
		return impl->rootElement.get();
	}

	UIElement* UIManager::getElement(uint32_t id) const {
		auto it = impl->elements.find(id);
		if (it == impl->elements.end()) return nullptr;

		return it->second;
	}

	size_t UIManager::getElementsCount() const {
		return impl->elements.size();
	}

	std::vector<UIElement*> UIManager::getElements() const {
		std::vector<UIElement*> elements;
		for (auto& kvp : impl->elements) {
			elements.push_back(kvp.second);
		}
		return elements;
	}

	void UIManager::deleteElement(uint32_t id) {
		auto it = impl->elements.find(id);
		if (it == impl->elements.end()) return;

		auto element = it->second;

		if (element->getParent()) {
			element->getParent()->deleteChild(id);
		}
	}

	void UIManager::deleteElement(UIElement* element) {
		if (element->impl->ctx == impl->ctx && element->impl->scene == impl->scene) {
			deleteElement(element->getId());
		}
	}

	void UIManager::setDefaultTextGlyph(Texture* glyph) {
		if (glyph->getContext() == impl->ctx)
			impl->defaultTextGlyph = glyph->getId();
	}

	void UIManager::setDefaultTextGlyph(uint32_t glyph) {
		impl->defaultTextGlyph = glyph;
	}

	uint32_t UIManager::getDefaultTextGlyph() const {
		return impl->defaultTextGlyph;
	}
}
