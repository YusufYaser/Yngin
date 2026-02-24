#pragma once
#include <Yngin/Yngin.h>
#include <Yngin/Scenes.h>
#include <memory>
#include <glm/vec3.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace Yngin {
	class Camera {
	public:
		glm::vec3 getPos() const;
		glm::vec3 getOrientation() const;
		float getFov() const;

		void setPos(glm::vec3 newPos);
		void setOrientation(glm::vec3 newOrientation);
		void setFov(float newFov);

		void lookAt(glm::vec3 target);

		glm::mat4 getView();
		glm::mat4 getProjection();

		float getWeight();
		void setWeight(float newWeight);

	private:
		friend class CamerasManager;
		friend struct std::default_delete<Camera>;

		Camera(Context* ctx, Scene* scene);
		~Camera();

		struct Impl;
		std::unique_ptr<Impl> impl;
	};

	class CamerasManager {
	public:
		uint32_t createCamera();
		void deleteCamera(uint32_t cameraId);

		Camera* getCamera(uint32_t cameraId);

		float getTotalWeight();

		// sets all cameras' weights to zero and this cameraId to 1
		void setActive(uint32_t cameraId);

		glm::vec3 getFinalPos();
		glm::vec3 getFinalOrientation();
		float getFinalFov();

		glm::mat4 getFinalView();
		glm::mat4 getFinalProjection();

	private:
		friend class Scene;
		friend struct std::default_delete<CamerasManager>;

		CamerasManager(Context* ctx, Scene* scene);
		~CamerasManager();

		struct Impl;
		std::unique_ptr<Impl> impl;
	};
}
