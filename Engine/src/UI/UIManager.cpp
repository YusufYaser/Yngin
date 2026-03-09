#include <Yngin/UI/UIManager.h>
#include "UI_Internal.h"
#include "Elements/UI_Elements_Internal.h"
#include <assert.h>

namespace Yngin {
	namespace UI {
		UIManager::UIManager(Context* ctx, Scene* scene) {
			impl = std::make_unique<Impl>();
			impl->ctx = ctx;
			impl->scene = scene;
			impl->rootElement = std::unique_ptr<UIElement>(new UIElement(ctx, scene, this, nullptr));
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

		void UIManager::deleteElement(uint32_t id) {
			auto it = impl->elements.find(id);
			if (it == impl->elements.end()) return;

			auto element = it->second;

			impl->elements.erase(id);

			if (element->getParent()) {
				element->getParent()->deleteChild(id);
			}
		}

		void UIManager::deleteElement(UIElement* element) {
			if (element->impl->ctx == impl->ctx && element->impl->scene == impl->scene) {
				deleteElement(element->getId());
			}
		}
	}
}
