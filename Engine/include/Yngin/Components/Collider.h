#pragma once
#include <Yngin/Forward.h>
#include <memory>
#include <glm/vec3.hpp>
#include "Component.h"

namespace Yngin {
	enum class COLLIDER_TYPE : uint8_t {
		NONE,
		BOX
	};

	namespace Components {
		class Collider : public Component {
		public:
			static const COLLIDER_TYPE staticType = COLLIDER_TYPE::NONE;
			virtual COLLIDER_TYPE getType() const;

			bool checkCollision(Collider* collider, bool fast = false) const;
			bool isPointInCollider(glm::vec3 point) const;

		private:
			Collider(GameObject* gameObject);
			~Collider();

			friend class GameObject;
			friend struct std::default_delete<Collider>;
			friend class BoxCollider;
			friend class Physics::PhysicsEngine;
		};

		class BoxCollider : public Collider {
		public:
			static const COLLIDER_TYPE staticType = COLLIDER_TYPE::BOX;
			COLLIDER_TYPE getType() const;

			void setSize(glm::vec3 size);
			glm::vec3 getSize();

			void setOffset(glm::vec3 offset);
			glm::vec3 getOffset();

		private:
			BoxCollider(GameObject* gameObject);
			~BoxCollider();

			friend class GameObject;
			friend struct std::default_delete<BoxCollider>;
			friend class Physics::PhysicsEngine;

			struct Impl;
			std::unique_ptr<Impl> impl;
		};
	}
}
