#pragma once
#include <memory>

namespace Yngin {
	class Context;
	class Scene;
	class Texture;

	namespace UI {
		class UIElement;

		class UIManager {
		public:
			UIElement* getRootElement() const;
			UIElement* getElement(uint32_t id) const;

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

