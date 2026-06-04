#pragma once
#include <Yngin/Forward.h>
#include <memory>
#include <glm/vec3.hpp>
#include "Component.h"

namespace Yngin {
	namespace Components {
		enum class LIGHT_TYPE : uint8_t {
			NONE,
			POINT,
			DIRECTIONAL
		};

		class Light : public Component {
		public:
			static const LIGHT_TYPE staticLightType = LIGHT_TYPE::NONE;
			virtual LIGHT_TYPE getLightType() const;

			glm::vec3 getColor() const;
			void setColor(glm::vec3 color);

			float getIntensity() const;
			void setIntensity(float intensity);

		private:
			Light(GameObject* gameObject);
			~Light();

			friend class GameObject;
			friend struct std::default_delete<Light>;
			friend class PointLight;
			friend class DirectionalLight;

			struct Impl;
			std::unique_ptr<Impl> impl;
		};

		class PointLight : public Light {
		public:
			static const COMPONENT_TYPE staticType = COMPONENT_TYPE::POINT_LIGHT;
			COMPONENT_TYPE getType() const;

			static const LIGHT_TYPE staticLightType = LIGHT_TYPE::POINT;
			LIGHT_TYPE getLightType() const;

			float getDistance() const;
			void setDistance(float distance);

		private:
			PointLight(GameObject* gameObject);
			~PointLight();

			friend class GameObject;
			friend struct std::default_delete<PointLight>;

			struct Impl;
			std::unique_ptr<Impl> impl;
		};

		class DirectionalLight : public Light {
		public:
			static const COMPONENT_TYPE staticType = COMPONENT_TYPE::DIRECTIONAL_LIGHT;
			COMPONENT_TYPE getType() const;

			static const LIGHT_TYPE staticLightType = LIGHT_TYPE::DIRECTIONAL;
			LIGHT_TYPE getLightType() const;

		private:
			DirectionalLight(GameObject* gameObject);
			~DirectionalLight();

			friend class GameObject;
			friend struct std::default_delete<DirectionalLight>;

			struct Impl;
			std::unique_ptr<Impl> impl;
		};
	}
}
