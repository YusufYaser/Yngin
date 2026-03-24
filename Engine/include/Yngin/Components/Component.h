#pragma once
#include <Yngin/Forward.h>
#include <memory>

namespace Yngin {
	namespace Components {
		class Component {
		public:
			GameObject* getGameObject();

		private:
			virtual void onRender();

			friend class GameObject;
			friend struct std::default_delete<Component>;
			friend class Rendering::Renderer;

		protected:
			Component(GameObject* gameObject);
			~Component();

			friend class Collider;
			friend class BoxCollider;
			friend class Physics::PhysicsEngine;

			struct Impl;
			std::unique_ptr<Impl> impl;
		};

		class Mesh;
	};
}