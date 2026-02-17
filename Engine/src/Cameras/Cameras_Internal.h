#pragma once
#include <Yngin/Yngin.h>

namespace Yngin {
	struct Camera::Impl {
		Context* ctx;
		Scene* scene;

		float fov = 70.0f;
		glm::vec3 pos = { 0, 0, 0 };
		glm::vec3 orientation = { 1, 0, 0 };

		float weight;
	};

	struct CamerasManager::Impl {
		Context* ctx;
		Scene* scene;

		std::map<uint32_t, std::unique_ptr<Camera>> cameras;
		uint32_t nextCameraId = 0;
	};
}
