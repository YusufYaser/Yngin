#pragma once
#include <Yngin/Yngin.h>
#include <Yngin/Scenes.h>
#include <memory>
#include <glm/vec3.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace Yngin {
	class Camera {
	public:
		Camera(Context* ctx, Scene* scene);
		~Camera();

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
		struct Impl;
		std::unique_ptr<Impl> impl;
	};

	class CamerasManager {
	public:
		// Also creates a default camera with cameraId 0
		CamerasManager(Context* ctx, Scene* scene);
		~CamerasManager();

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
		Context* ctx;
		Scene* scene;

		std::map<uint32_t, std::unique_ptr<Camera>> cameras;
		uint32_t nextCameraId = 0;
	};
}
