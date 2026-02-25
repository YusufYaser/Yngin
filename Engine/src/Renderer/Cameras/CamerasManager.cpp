#include <Yngin/Renderer/Cameras.h>
#include "Cameras_Internal.h"
#include "../../Core/Scenes/Scenes_Internal.h"
#include <assert.h>
#include <stdexcept>

namespace Yngin {
	CamerasManager::CamerasManager(Context* ctx, Scene* scene) {
		if (scene->getCamerasManager()) {
			throw std::invalid_argument("Scene already has a cameras manager!");
		}

		impl = std::make_unique<Impl>();

		impl->ctx = ctx;
		impl->scene = scene;

		uint32_t defaultCamera = createCamera();
		getCamera(defaultCamera)->setWeight(1.0f);
	}

	CamerasManager::~CamerasManager() {
	}

	uint32_t CamerasManager::createCamera() {
		auto camera = std::unique_ptr<Camera>(new Camera(impl->ctx, impl->scene));

		uint32_t cameraId = impl->nextCameraId++;
		impl->cameras[cameraId] = std::move(camera);
		return cameraId;
	}

	void CamerasManager::deleteCamera(uint32_t cameraId) {
		assert(cameraId != 0);

		if (cameraId == 0) return;

		impl->cameras.erase(cameraId);
	}

	Camera* CamerasManager::getCamera(uint32_t cameraId) {
		auto it = impl->cameras.find(cameraId);
		assert(it != impl->cameras.end());

		if (it == impl->cameras.end()) return nullptr;

		return it->second.get();
	}

	float CamerasManager::getTotalWeight() {
		float totalWeight = 0.0f;

		for (auto& kvp : impl->cameras) {
			totalWeight += kvp.second->getWeight();
		}

		return totalWeight;
	}

	void CamerasManager::setActive(uint32_t cameraId) {
		Camera* camera = getCamera(cameraId);
		if (camera == nullptr) return;

		for (auto& kvp : impl->cameras) {
			kvp.second->setWeight(0.0f);
		}

		camera->setWeight(1.0f);
	}

	glm::vec3 CamerasManager::getFinalPos() {
		glm::vec3 finalPos = {};
		for (auto& kvp : impl->cameras) {
			// pos * weight
			finalPos += kvp.second->getPos() * kvp.second->getWeight();
		}
		finalPos /= getTotalWeight();
		return finalPos;
	}

	glm::vec3 CamerasManager::getFinalOrientation() {
		glm::vec3 finalOrientation = {};
		for (auto& kvp : impl->cameras) {
			// orientation * weight
			finalOrientation += kvp.second->getOrientation() * kvp.second->getWeight();
		}
		finalOrientation /= getTotalWeight();
		return finalOrientation;
	}

	float CamerasManager::getFinalFov() {
		float finalFov = 0.0f;
		for (auto& kvp : impl->cameras) {
			// fov * weight
			finalFov += kvp.second->getFov() * kvp.second->getWeight();
		}
		finalFov /= getTotalWeight();
		return finalFov;
	}

	glm::mat4 CamerasManager::getFinalView() {
		glm::vec3 pos = getFinalPos();
		glm::vec3 orientation = getFinalOrientation();

		return glm::lookAt(pos, pos + orientation, { 0, 0, 1 });
	}

	glm::mat4 CamerasManager::getFinalProjection() {
		glm::ivec2 viewportSize = impl->ctx->getViewportSize();
		float aspectRatio = 1.0f;
		if (viewportSize.y != 0) {
			aspectRatio = viewportSize.x * 1.0f / viewportSize.y;
		}

		float fov = getFinalFov();

		return glm::perspective(fov, aspectRatio, 0.1f, 1000.0f);
	}
}
