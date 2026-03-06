#pragma once
#include <memory>

namespace Yngin {
	class Context;
	class Scene;

	namespace UI {
		class UIElement;

		class UIManager {
		public:
			UIElement* getRootElement();
			UIElement* getElement(uint32_t id);

			void deleteElement(uint32_t id);
			void deleteElement(UIElement* element);

		private:
			friend class Scene;
			friend struct std::default_delete<UIManager>;
			friend class UIElement;

			UIManager(Context* ctx, Scene* scene);
			~UIManager();

			uint32_t acquireId();

			struct Impl;
			std::unique_ptr<Impl> impl;
		};
	}
}

