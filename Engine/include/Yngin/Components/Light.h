#pragma once
#include <memory>
#include <glm/vec3.hpp>
#include "Component.h"

namespace Yngin {
	namespace Components {
		class Light : public Component {
		public:
			glm::vec3 getColor();
			void setColor(glm::vec3 color);

			float getDistance();
			void setDistance(float distance);

			float getIntensity();
			void setIntensity(float intensity);

		private:
			Light(GameObject* gameObject);
			~Light();

			friend class GameObject;
			friend struct std::default_delete<Light>;

			struct Impl;
			std::unique_ptr<Impl> impl;
		};
	}
}
