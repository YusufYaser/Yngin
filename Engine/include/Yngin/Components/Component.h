#pragma once
#include <memory>

namespace Yngin {
	class Context;
	class GameObject;

	namespace Components {
		class Component {
		private:
			virtual void onRender();

			friend class GameObject;
			friend struct std::default_delete<Component>;
			friend class Mesh;

			Component(GameObject* gameObject);
			~Component();

		protected:
			struct Impl;
			std::unique_ptr<Impl> impl;
		};

		class Mesh;
	};
}