#pragma once
#include <Yngin/Forward.h>
#include <memory>

namespace Yngin {
	enum class COMPONENT_TYPE : uint8_t {
		NONE = 0,
		MESH,
		POINT_LIGHT,
		DIRECTIONAL_LIGHT,
		BOX_COLLIDER,
		RIGID_BODY,
		COUNT
	};

	namespace Components {
		class Component {
		public:
			static const COMPONENT_TYPE staticType = COMPONENT_TYPE::NONE;
			virtual COMPONENT_TYPE getType() const;

			GameObject* getGameObject();

		private:
			virtual void onRender();

			friend class GameObject;
			friend struct std::default_delete<Component>;
			friend class Rendering::Renderer;

		protected:
			Component(GameObject* gameObject);
			virtual ~Component();

			friend class Collider;
			friend class BoxCollider;
			friend class Physics::PhysicsEngine;

			struct Impl;
			std::unique_ptr<Impl> impl;
		};

		class Mesh;
	};
}