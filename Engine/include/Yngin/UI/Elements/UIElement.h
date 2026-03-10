#pragma once
#include <memory>
#include <glm/vec2.hpp>
#include <glm/vec4.hpp>
#include <Yngin/Core/InputSystem.h>

namespace Yngin {
	class Context;
	class Scene;

	enum class UI_TYPE : uint8_t {
		NONE,
		IMAGE,
		TEXT,
		BUTTON
	};

	namespace UI {
		struct UITransform {
			float xScale;
			int xOffset;
			float yScale;
			int yOffset;
		};

		struct UICrop {
			glm::vec2 start = glm::vec2(0.0f);
			glm::vec2 end = glm::vec2(1.0f);
		};

		class UIElement {
		public:
			static const UI_TYPE staticType = UI_TYPE::NONE;

			uint32_t getId() const;
			Context* getContext() const;
			Scene* getScene() const;

			virtual UI_TYPE getType() const;

			UIElement* getParent() const;
			template<typename T>
			T* getParent() const;
			void setParent(UIElement* newParent);

			UIElement* getChild(uint32_t childId) const;
			template<typename T>
			T* getChild(uint32_t childId) const;

			UIElement* createChild();
			template<typename T>
			T* createChild();

			void deleteChild(uint32_t childId);
			void deleteChild(UIElement* child);

			void moveChild(uint32_t childId, UIElement* newParent);
			void moveChild(UIElement* child, UIElement* newParent);
			void moveChild(UIElement* child, uint32_t newParentId);
			void moveChild(uint32_t childId, uint32_t newParentId);

			void setPosition(UITransform newPos);
			UITransform getPosition() const;

			void setSize(UITransform newSize);
			UITransform getSize() const;

			bool isHovered() const;
			bool isClicked(const Yngin::MOUSE_BUTTON& btn = Yngin::MOUSE_BUTTON::LEFT) const;
			bool isHeld(const Yngin::MOUSE_BUTTON& btn = Yngin::MOUSE_BUTTON::LEFT) const;

			virtual void setCrop(UICrop newCrop);
			virtual UICrop getCrop() const;

			void setColor(glm::vec4 newColor);
			glm::vec4 getColor() const;

			virtual void setPivot(glm::vec2 newPivot);
			virtual glm::vec2 getPivot() const;

		private:
			friend class UIManager;
			friend struct std::default_delete<UIElement>;
			friend class Scene;
			friend class Context;

			friend class Image;
			friend class Text;
			friend class Button;

			UIElement(Context* ctx, Scene* scene, UIManager* mgr, UIElement* parent);
			~UIElement();

			virtual void render();

			struct Impl;
			std::unique_ptr<Impl> impl;
		};
	}
}
