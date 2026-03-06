#pragma once
#include <memory>
#include <glm/vec3.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "../Services/Tween.h"

namespace Yngin {
	class Context;
	class Scene;

	class Camera {
	public:
		uint32_t getId();
		Context* getContext();
		Scene* getScene();

		glm::vec3 getPos() const;
		glm::vec3 getOrientation() const;
		float getFov() const;

		void setPos(glm::vec3 newPos);
		void setOrientation(glm::vec3 newOrientation);
		void setFov(float newFov);

		void lookAt(glm::vec3 target);

		float getWeight();
		void setWeight(float newWeight);

	private:
		friend class CamerasManager;
		friend struct std::default_delete<Camera>;
		friend class Services::Tween;

		Camera(Context* ctx, Scene* scene);
		~Camera();

		struct Impl;
		std::unique_ptr<Impl> impl;
	};

	class CamerasManager {
	public:
		Camera* createCamera();
		void deleteCamera(uint32_t cameraId);
		void deleteCamera(Camera* camera);

		Camera* getCamera(uint32_t cameraId);

		float getTotalWeight();

		// sets all cameras' weights to zero and this cameraId to 1
		void setActive(uint32_t cameraId);
		void setActive(Camera* camera);

		glm::vec3 getFinalPos();
		glm::vec3 getFinalOrientation();
		float getFinalFov();

	private:
		friend class Scene;
		friend struct std::default_delete<CamerasManager>;

		CamerasManager(Scene* scene);
		~CamerasManager();

		struct Impl;
		std::unique_ptr<Impl> impl;
	};
}
