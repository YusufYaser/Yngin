#include <Yngin/Rendering/Cameras.h>
#include "Cameras_Internal.h"
#include "../../Core/Scenes/Scenes_Internal.h"
#include <assert.h>
#include <stdexcept>

namespace Yngin {
	CamerasManager::CamerasManager(Scene* scene) {
		if (scene->getCamerasManager()) {
			throw std::invalid_argument("Scene already has a cameras manager!");
		}

		impl = std::make_unique<Impl>();

		impl->ctx = scene->getContext();
		impl->scene = scene;
		impl->owner = this;

		Camera* defaultCamera = createCamera();
		defaultCamera->setWeight(1.0f);
	}

	CamerasManager::~CamerasManager() {
	}

	Camera* CamerasManager::createCamera() {
		auto camera = std::unique_ptr<Camera>(new Camera(impl->ctx, impl->scene));

		uint32_t cameraId = impl->nextCameraId++;
		camera->impl->id = cameraId;
		impl->cameras[cameraId] = std::move(camera);
		return impl->cameras[cameraId].get();
	}

	void CamerasManager::deleteCamera(uint32_t cameraId) {
		assert(cameraId != 0);

		if (cameraId == 0) return;

		impl->cameras.erase(cameraId);
	}

	void CamerasManager::deleteCamera(Camera* camera) {
		if (camera->impl->scene == impl->scene) {
			deleteCamera(camera->impl->id);
		}
	}

	Camera* CamerasManager::getCamera(uint32_t cameraId) const {
		auto it = impl->cameras.find(cameraId);
		if (it == impl->cameras.end()) return nullptr;

		return it->second.get();
	}

	float CamerasManager::getTotalWeight() const {
		float totalWeight = 0.0f;

		for (auto& kvp : impl->cameras) {
			totalWeight += kvp.second->getWeight();
		}

		return totalWeight;
	}

	void CamerasManager::setActive(uint32_t cameraId) {
		Camera* camera = getCamera(cameraId);
		if (camera == nullptr) return;
		setActive(camera);
	}

	void CamerasManager::setActive(Camera* camera) {
		if (camera->impl->scene != impl->scene) return;

		for (auto& kvp : impl->cameras) {
			kvp.second->setWeight(0.0f);
		}

		camera->setWeight(1.0f);
	}

	glm::vec3 CamerasManager::getFinalPos() const {
		glm::vec3 finalPos = {};
		for (auto& kvp : impl->cameras) {
			// pos * weight
			finalPos += kvp.second->getPosition() * kvp.second->getWeight();
		}
		finalPos /= getTotalWeight();
		return finalPos;
	}

	glm::vec3 CamerasManager::getFinalOrientation() const {
		glm::vec3 finalOrientation = {};
		for (auto& kvp : impl->cameras) {
			// orientation * weight
			finalOrientation += kvp.second->getOrientation() * kvp.second->getWeight();
		}
		finalOrientation /= getTotalWeight();
		return finalOrientation;
	}

	float CamerasManager::getFinalFov() const {
		float finalFov = 0.0f;
		for (auto& kvp : impl->cameras) {
			// fov * weight
			finalFov += kvp.second->getFov() * kvp.second->getWeight();
		}
		finalFov /= getTotalWeight();
		return finalFov;
	}

	glm::mat4 CamerasManager::Impl::getFinalView() {
		glm::vec3 pos = owner->getFinalPos();
		glm::vec3 orientation = owner->getFinalOrientation();

		return glm::lookAt(pos, pos + orientation, { 0, 0, 1 });
	}

	glm::mat4 CamerasManager::Impl::getFinalPerspectiveProjection() {
		glm::ivec2 viewportSize = ctx->getViewportSize();
		float aspectRatio = 1.0f;
		if (viewportSize.y != 0) {
			aspectRatio = viewportSize.x * 1.0f / viewportSize.y;
		}

		float fov = owner->getFinalFov();

		return glm::perspective(glm::radians(fov), aspectRatio, 0.1f, 1000.0f);
	}
}
