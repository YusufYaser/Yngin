#include <Yngin/UI/UI.h>
#include "UI_Elements_Internal.h"
#include "../UI_Internal.h"
#include <assert.h>
#include <Yngin/Core/Scenes.h>
#include <Yngin/Core/Context.h>
#include <Yngin/Core/InputSystem.h>
#include <Yngin/Renderer/Shaders.h>

namespace Yngin {
	namespace UI {
		UIElement::UIElement(Context* ctx, Scene* scene, UIManager* mgr, UIElement* parent) {
			impl = std::make_unique<Impl>();

			impl->ctx = ctx;
			impl->scene = scene;
			impl->mgr = mgr;
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

		bool UIElement::isHovered() {
			auto& m = impl;

			glm::ivec2 screenSize = impl->ctx->getViewportSize();

			glm::ivec2 size = {
				m->size.xOffset + int(m->size.xScale * screenSize.x),
				m->size.yOffset + int(m->size.yScale * screenSize.y)
			};

			glm::ivec2 boundsStart = {
				m->pos.xOffset + int(m->pos.xScale * screenSize.x) - size.x / 2,
				m->pos.yOffset + int(m->pos.yScale * screenSize.y) - size.y / 2
			};

			glm::ivec2 boundsEnd = boundsStart + size;

			glm::ivec2 mousePos = impl->ctx->getInputSystem()->getMousePos();

			bool x = boundsStart.x <= mousePos.x && mousePos.x <= boundsEnd.x;
			bool y = boundsStart.y <= mousePos.y && mousePos.y <= boundsEnd.y;

			return x && y;
		}

		bool UIElement::isClicked(Yngin::MOUSE_BUTTON btn) {
			return isHovered() && impl->ctx->getInputSystem()->isMouseJustPressed(btn);
		}

		bool UIElement::isHeld(Yngin::MOUSE_BUTTON btn) {
			return isHovered() && impl->ctx->getInputSystem()->isMousePressed(btn);
		}

		void UIElement::setCrop(UICrop newCrop) {
			impl->crop = newCrop;
		}

		UICrop UIElement::getCrop() {
			return impl->crop;
		}

		void UIElement::setColor(glm::vec4 newColor) {
			impl->color = newColor;
		}

		glm::vec4 UIElement::getColor() {
			return impl->color;
		}

		void UIElement::setPivot(glm::vec2 newPivot) {
			impl->pivot = newPivot;
		}

		glm::vec2 UIElement::getPivot() {
			return impl->pivot;
		}

		template<typename T>
		T* UIElement::getParent() {
			if (impl->parent == nullptr) return nullptr;

			assert(impl->parent->getType() == T::staticType);

			if (impl->parent->getType() != T::staticType) return nullptr;

			return dynamic_cast<T*>(impl->parent);
		}

		template Image* UIElement::getParent<Image>();
		template Text* UIElement::getParent<Text>();

		void UIElement::setParent(UIElement* newParent) {
			impl->parent->moveChild(impl->id, newParent);
		}

		template<typename T>
		T* UIElement::createChild() {
			auto element = std::unique_ptr<T>(new T(impl->ctx, impl->scene, impl->mgr, this));

			uint32_t id = impl->mgr->acquireId();
			dynamic_cast<UIElement*>(element.get())->impl->id = id;

			impl->childs[id] = std::move(element);
			T* obj = dynamic_cast<T*>(impl->childs[id].get());
			impl->mgr->impl->elements[id] = obj;

			return obj;
		}

		template Image* UIElement::createChild<Image>();
		template Text* UIElement::createChild<Text>();

		UIElement* UIElement::getChild(uint32_t childId) {
			auto it = impl->childs.find(childId);
			if (it == impl->childs.end()) return nullptr;

			return it->second.get();
		}

		template<typename T>
		T* UIElement::getChild(uint32_t childId) {
			UIElement* child = getChild(childId);

			if (child == nullptr) return nullptr;

			assert(child->getType() == T::staticType);

			if (child->getType() != T::staticType) return nullptr;

			return dynamic_cast<T*>(child);
		}

		template Image* UIElement::getChild<Image>(uint32_t);
		template Text* UIElement::getChild<Text>(uint32_t);

		void UIElement::deleteChild(uint32_t childId) {
			UIElement* child = getChild(childId);
			if (child) deleteChild(child);
		}

		void UIElement::deleteChild(UIElement* child) {
			uint32_t childId = child->getId();
			impl->childs.erase(childId);
			impl->scene->getUIManager()->impl->elements.erase(childId);
		}

		void UIElement::moveChild(uint32_t childId, UIElement* newParent) {
			assert(newParent->getContext() == impl->ctx);
			assert(newParent->getScene() == impl->scene);

			if (newParent->getContext() != impl->ctx) return;
			if (newParent->getScene() != impl->scene) return;

			auto it = impl->childs.find(childId);
			if (it == impl->childs.end()) return;

			newParent->impl->childs[childId] = std::move(it->second);
			impl->childs.erase(childId);
		}

		void UIElement::moveChild(UIElement* child, UIElement* newParent) {
			moveChild(child->getId(), newParent);
		}

		void UIElement::moveChild(UIElement* child, uint32_t newParentId) {
			UIElement* newParent = impl->scene->getUIManager()->getElement(newParentId);
			if (newParent) moveChild(child->getId(), newParent);
		}

		void UIElement::moveChild(uint32_t childId, uint32_t newParentId) {
			UIElement* newParent = impl->scene->getUIManager()->getElement(newParentId);
			if (newParent) moveChild(childId, newParent);
		}

		void UIElement::Impl::prepareUniforms() {
			Shader* uiShader = ctx->getShadersManager()->getShader(SHADER_TYPE::UI);
			uiShader->activate();

			glm::ivec2 screenSize = ctx->getViewportSize();

			glm::ivec2 sizePixels = {
				size.xScale * screenSize.x + size.xOffset,
				size.yScale * screenSize.y + size.yOffset,
			};

			glm::ivec2 pivotOffset = {
				sizePixels.x * (pivot.x - 0.5f),
				sizePixels.y * (pivot.y - 0.5f)
			};

			uiShader->setFloat("uiPosition.xScale", pos.xScale);
			uiShader->setInt("uiPosition.xOffset", pos.xOffset - pivotOffset.x);
			uiShader->setFloat("uiPosition.yScale", pos.yScale);
			uiShader->setInt("uiPosition.yOffset", pos.yOffset - pivotOffset.y);

			uiShader->setFloat("uiSize.xScale", size.xScale);
			uiShader->setInt("uiSize.xOffset", size.xOffset);
			uiShader->setFloat("uiSize.yScale", size.yScale);
			uiShader->setInt("uiSize.yOffset", size.yOffset);

			uiShader->setIVec2("screenSize", screenSize);

			uiShader->setVec2("uiCrop.start", crop.start);
			uiShader->setVec2("uiCrop.end", crop.end);

			uiShader->setVec4("color", color);
		}

		void UIElement::render() {
			for (auto& kvp : impl->childs) {
				kvp.second->render();
			}
		}
	}
}
