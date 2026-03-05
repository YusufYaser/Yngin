#include <Yngin/UI/UI.h>
#include "UI_Elements_Internal.h"
#include "../UI_Internal.h"
#include <assert.h>
#include <Yngin/Core/Scenes.h>

namespace Yngin {
	namespace UI {
		UIElement::UIElement(Context* ctx, Scene* scene, UIElement* parent) {
			impl = std::make_unique<Impl>();

			impl->ctx = ctx;
			impl->scene = scene;
			impl->parent = parent;
		}

		UIElement::~UIElement() = default;

		UIType UIElement::getType() {
			return UIType::None;
		}

		uint32_t UIElement::getId() {
			return impl->id;
		}

		Context* UIElement::getContext() {
			return impl->ctx;
		}

		Scene* UIElement::getScene() {
			return impl->scene;
		}

		UIElement* UIElement::getParent() {
			return impl->parent;
		}

		void UIElement::setPos(UITransform newPos) {
			impl->pos = newPos;
		}

		UITransform UIElement::getPos() {
			return impl->pos;
		}

		void UIElement::setSize(UITransform newSize) {
			impl->size = newSize;
		}

		UITransform UIElement::getSize() {
			return impl->size;
		}

		template<typename T>
		T* UIElement::getParent() {
			if (impl->parent == nullptr) return nullptr;

			assert(impl->parent->getType() == T::staticType);

			if (impl->parent->getType() != T::staticType) return nullptr;

			return impl->parent;
		}

		void UIElement::setParent(UIElement* newParent) {
			impl->parent->moveChild(impl->id, newParent);
		}

		template<typename T>
		T* UIElement::createChild() {
			auto element = std::unique_ptr<T>(new T(impl->ctx, impl->scene, this));

			uint32_t id = impl->scene->getUIManager()->acquireId();
			dynamic_cast<UIElement*>(element.get())->impl->id = id;

			impl->childs[id] = std::move(element);
			T* obj = dynamic_cast<T*>(impl->childs[id].get());
			impl->scene->getUIManager()->impl->elements[id] = obj;

			return obj;
		}

		template Image* UIElement::createChild<Image>();

		UIElement* UIElement::getChild(uint32_t childId) {
			auto it = impl->childs.find(childId);
			assert(it != impl->childs.end());

			if (it == impl->childs.end()) return nullptr;

			return it->second.get();
		}

		template<typename T>
		T* UIElement::getChild(uint32_t childId) {
			UIElement* child = getChild(childId);

			if (child == nullptr) return nullptr;

			assert(child->getType() == T::staticType);

			if (child->getType() != T::staticType) return nullptr;

			return child;
		}

		void UIElement::deleteChild(uint32_t childId) {
			impl->childs.erase(childId);
			impl->scene->getUIManager()->impl->elements.erase(childId);
		}

		void UIElement::moveChild(uint32_t childId, UIElement* newParent) {
			assert(newParent->getContext() == impl->ctx);
			assert(newParent->getScene() == impl->scene);

			if (newParent->getContext() != impl->ctx) return;
			if (newParent->getScene() != impl->scene) return;

			newParent->impl->childs[childId] = std::move(impl->childs[childId]);
			impl->childs.erase(childId);
		}

		void UIElement::render() {
			for (auto& kvp : impl->childs) {
				kvp.second->render();
			}
		}
	}
}
