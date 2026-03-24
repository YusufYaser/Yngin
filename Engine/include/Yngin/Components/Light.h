#pragma once
#include <Yngin/Forward.h>
#include <memory>
#include <glm/vec3.hpp>
#include "Component.h"

namespace Yngin {
	namespace Components {
		class Light : public Component {
		public:
			glm::vec3 getColor() const;
			void setColor(glm::vec3 color);

			float getDistance() const;
			void setDistance(float distance);

			float getIntensity() const;
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
