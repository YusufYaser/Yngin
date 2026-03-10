#pragma once
#include <memory>

namespace Yngin {
	class Context;
	class GameObject;

	namespace Components {
		class Component {
		public:
			GameObject* getGameObject();

		private:
			virtual void onRender();

			friend class GameObject;
			friend struct std::default_delete<Component>;

		protected:
			Component(GameObject* gameObject);
			~Component();

			friend class BoxCollider;

			struct Impl;
			std::unique_ptr<Impl> impl;
		};

		class Mesh;
	};
}