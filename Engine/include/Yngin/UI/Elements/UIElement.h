#pragma once
#include <memory>
#include <glm/vec2.hpp>
#include <Yngin/Core/InputSystem.h>

namespace Yngin {
	class Context;
	class Scene;

	namespace UI {
		enum class UIType : uint8_t {
			None,
			Image
		};

		struct UITransform {
			float xScale;
			int xOffset;
			float yScale;
			int yOffset;
		};

		class UIElement {
		public:
			uint32_t getId();
			Context* getContext();
			Scene* getScene();

			virtual UIType getType();

			UIElement* getParent();
			template<typename T>
			T* getParent();
			void setParent(UIElement* newParent);

			UIElement* getChild(uint32_t childId);
			template<typename T>
			T* getChild(uint32_t childId);
			template<typename T>
			T* createChild();
			void deleteChild(uint32_t childId);

			void moveChild(uint32_t childId, UIElement* newParent);

			void setPos(UITransform newPos);
			UITransform getPos();

			void setSize(UITransform newSize);
			UITransform getSize();

			bool isHovered();
			bool isClicked(Yngin::MOUSE_BUTTON btn = Yngin::MOUSE_BUTTON::LEFT);
			bool isHeld(Yngin::MOUSE_BUTTON btn = Yngin::MOUSE_BUTTON::LEFT);

		private:
			friend class UIManager;
			friend struct std::default_delete<UIElement>;
			friend class Image;
			friend class Scene;

			UIElement(Context* ctx, Scene* scene, UIElement* parent);
			~UIElement();

			virtual void render();

			struct Impl;
			std::unique_ptr<Impl> impl;
		};
	}
}
