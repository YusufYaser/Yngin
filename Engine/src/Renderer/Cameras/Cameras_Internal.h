#pragma once
#include <map>
#include <Yngin/Core/Context.h>

namespace Yngin {
	struct Camera::Impl {
		Context* ctx;
		Scene* scene;

		uint32_t id;

		float fov = 70.0f;
		glm::vec3 pos = { 0, 0, 0 };
		glm::vec3 orientation = { 1, 0, 0 };

		float weight;

		glm::mat4 getView();
		glm::mat4 getPerspectiveProjection();
	};

	struct CamerasManager::Impl {
		Context* ctx;
		Scene* scene;
		CamerasManager* owner;

		std::map<uint32_t, std::unique_ptr<Camera>> cameras;
		uint32_t nextCameraId = 0;

		glm::mat4 getFinalView();
		glm::mat4 getFinalPerspectiveProjection();
	};
}
