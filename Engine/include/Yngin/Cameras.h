#pragma once
#include <Yngin/Yngin.h>
#include <Yngin/Scenes.h>
#include <memory>
#include <glm/vec3.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace Yngin {
	class Camera;

	class CamerasManager {
	public:
		// Also creates a default camera with cameraId 0
		CamerasManager(Context* ctx, Scene* scene);
		~CamerasManager();

		uint32_t createCamera();
		void deleteCamera(uint32_t cameraId);

		float getTotalWeight();
		float getWeight(uint32_t cameraId);
		// camera weight / total weight
		float getRelativeWeight(uint32_t cameraId);
		void setWeight(uint32_t cameraId, float newWeight);

		// sets all cameras' weights to zero and this cameraId to 1
		void setActive(uint32_t cameraId);

		void setPos(uint32_t cameraId, glm::vec3 newPos);
		void setOrientation(uint32_t cameraId, glm::vec3 newOrientation);
		void setFov(uint32_t cameraId, float newFov);

		void lookAt(uint32_t cameraId, glm::vec3 target);

		glm::vec3 getFinalPos();
		glm::vec3 getFinalOrientation();
		float getFinalFov();

		glm::mat4 getFinalView();
		glm::mat4 getFinalProjection();

	private:
		Context* ctx;
		Scene* scene;

		std::map<uint32_t, std::pair<std::unique_ptr<Camera>, float>> cameras;
		uint32_t nextCameraId = 0;

		std::pair<std::unique_ptr<Camera>, float>& getCameraPair(uint32_t cameraId);
	};
}
