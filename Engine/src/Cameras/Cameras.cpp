#include <Yngin/Cameras.h>
#include "Cameras_Internal.h"
#include <assert.h>
#include <stdexcept>

namespace Yngin {
	Camera::Camera(Context* ctx, Scene* scene) : ctx(ctx), scene(scene) {

	}

	Camera::~Camera() {

	}

	void Camera::lookAt(glm::vec3 target) {
		glm::vec3 diff = target - pos;
		orientation = diff / glm::length(diff);
	}

	glm::mat4 Camera::getView() {
		return glm::lookAt(pos, pos + orientation, { 0, 0, 1 });
	}

	glm::mat4 Camera::getProjection() {
		glm::ivec2 viewportSize = ctx->getViewportSize();
		float aspectRatio = viewportSize.x * 1.0f / viewportSize.y;
		return glm::perspective(fov, aspectRatio, 0.1f, 1000.0f);
	}

	CamerasManager::CamerasManager(Context* ctx, Scene* scene) {
		if (scene->getCamerasManager()) {
			throw std::invalid_argument("Scene already has a cameras manager!");
		}

		this->ctx = ctx;
		this->scene = scene;

		createCamera();
		setWeight(0, 1.0f);
	}

	CamerasManager::~CamerasManager() {
	}

	uint32_t CamerasManager::createCamera() {
		uint32_t cameraId = nextCameraId++;
		cameras[cameraId] = std::make_pair(std::make_unique<Camera>(ctx, scene), 0);
		return cameraId;
	}

	void CamerasManager::deleteCamera(uint32_t cameraId) {
		assert(cameraId != 0);

		if (cameraId == 0) return;

		cameras.erase(cameraId);
	}

	float CamerasManager::getTotalWeight() {
		float totalWeight = 0.0f;

		for (auto& kvp : cameras) {
			totalWeight += kvp.second.second;
		}

		return totalWeight;
	}

	float CamerasManager::getWeight(uint32_t cameraId) {
		auto& [camera, weight] = getCameraPair(cameraId);
		if (camera == nullptr) return 0.0f;

		return weight;
	}

	float CamerasManager::getRelativeWeight(uint32_t cameraId) {
		return getWeight(cameraId) / getTotalWeight();
	}

	void CamerasManager::setWeight(uint32_t cameraId, float newWeight) {
		auto& [camera, weight] = getCameraPair(cameraId);
		if (camera == nullptr) return;

		weight = newWeight;
	}

	void CamerasManager::setActive(uint32_t cameraId) {
		auto& [camera, weight] = getCameraPair(cameraId);
		if (camera == nullptr) return;

		for (auto& kvp : cameras) {
			kvp.second.second = 0;
		}

		weight = 1.0f;
	}

	void CamerasManager::setPos(uint32_t cameraId, glm::vec3 newPos) {
		auto& [camera, weight] = getCameraPair(cameraId);
		if (camera == nullptr) return;

		camera->setPos(newPos);
	}

	void CamerasManager::setOrientation(uint32_t cameraId, glm::vec3 newOrientation) {
		auto& [camera, weight] = getCameraPair(cameraId);
		if (camera == nullptr) return;

		camera->setOrientation(newOrientation);
	}

	void CamerasManager::setFov(uint32_t cameraId, float newFov) {
		auto& [camera, weight] = getCameraPair(cameraId);
		if (camera == nullptr) return;

		camera->setFov(newFov);
	}

	void CamerasManager::lookAt(uint32_t cameraId, glm::vec3 target) {
		auto it = cameras.find(cameraId);
		assert(it != cameras.end());
		if (it == cameras.end()) return;

		it->second.first->lookAt(target);
	}

	glm::vec3 CamerasManager::getFinalPos() {
		glm::vec3 finalPos = {};
		for (auto& kvp : cameras) {
			// pos * weight
			finalPos += kvp.second.first->getPos() * kvp.second.second;
		}
		finalPos /= getTotalWeight();
		return finalPos;
	}

	glm::vec3 CamerasManager::getFinalOrientation() {
		glm::vec3 finalOrientation = {};
		for (auto& kvp : cameras) {
			// orientation * weight
			finalOrientation += kvp.second.first->getOrientation() * kvp.second.second;
		}
		finalOrientation /= getTotalWeight();
		return finalOrientation;
	}

	float CamerasManager::getFinalFov() {
		float finalFov = 0.0f;
		for (auto& kvp : cameras) {
			// fov * weight
			finalFov += kvp.second.first->getFov() * kvp.second.second;
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
		glm::ivec2 viewportSize = ctx->getViewportSize();
		float aspectRatio = viewportSize.x * 1.0f / viewportSize.y;

		float fov = getFinalFov();

		return glm::perspective(fov, aspectRatio, 0.1f, 1000.0f);
	}

	std::pair<std::unique_ptr<Camera>, float>& CamerasManager::getCameraPair(uint32_t cameraId) {
		auto it = cameras.find(cameraId);
		assert(it != cameras.end());

		static std::pair<std::unique_ptr<Camera>, float> nullPair{ nullptr, 0.0f };

		if (it == cameras.end()) return nullPair;

		return it->second;
	}
}
