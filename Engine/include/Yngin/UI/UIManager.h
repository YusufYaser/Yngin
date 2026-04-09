#pragma once
#include <Yngin/Forward.h>
#include <memory>
#include <vector>

namespace Yngin {
	namespace UI {
		class UIManager {
		public:
			Context* getContext() const;

			UIElement* getRootElement() const;
			UIElement* getElement(uint32_t id) const;

			size_t getElementsCount() const;
			std::vector<UIElement*> getElements() const;

			void deleteElement(uint32_t id);
			void deleteElement(UIElement* element);

			void setDefaultTextGlyph(Texture* tex);
			void setDefaultTextGlyph(uint32_t tex);
			uint32_t getDefaultTextGlyph() const;

		private:
			friend class Scene;
			friend struct std::default_delete<UIManager>;
			friend class UIElement;
			friend class Context;

			UIManager(Context* ctx, Scene* scene);
			~UIManager();

			uint32_t acquireId();

			struct Impl;
			std::unique_ptr<Impl> impl;
		};
	}
}

