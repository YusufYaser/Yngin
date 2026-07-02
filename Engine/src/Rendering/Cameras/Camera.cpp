#include <Yngin/Rendering/Cameras.h>
#include "Cameras_Internal.h"

namespace Yngin {
	Camera::Camera(Context* ctx, Scene* scene) {
		impl = std::make_unique<Impl>();
		impl->ctx = ctx;
		impl->scene = scene;
		impl->owner = this;
	}

	Camera::~Camera() = default;

	uint32_t Camera::getId() const {
		return impl->id;
	}

	Context* Camera::getContext() const {
		return impl->ctx;
	}

	Scene* Camera::getScene() const {
		return impl->scene;
	}

	glm::vec3 Camera::getPosition() const {
		return impl->pos;
	}

	glm::vec3 Camera::getOrientation() const {
		return impl->orientation;
	}

	float Camera::getFov() const {
		return impl->fov;
	}

	void Camera::setPosition(glm::vec3 newPos) {
		impl->pos = newPos;
	}

	void Camera::setOrientation(glm::vec3 newOrientation) {
		impl->orientation = newOrientation;
	}

	void Camera::setFov(float newFov) {
		impl->fov = newFov;
	}

	void Camera::lookAt(glm::vec3 target) {
		glm::vec3 diff = target - impl->pos;
		impl->orientation = diff / glm::length(diff);
	}

	glm::mat4 Camera::Impl::getView() {
		glm::vec3 pos = owner->getPosition();
		glm::vec3 orientation = owner->getOrientation();

		return glm::lookAt(pos, pos + orientation, { 0, 0, 1 });
	}

	glm::mat4 Camera::Impl::getPerspectiveProjection() {
		float fov = owner->getFov();

		glm::ivec2 viewportSize = ctx->getViewportSize();
		float aspectRatio = viewportSize.x * 1.0f / viewportSize.y;
		return glm::perspective(glm::radians(fov), aspectRatio, 0.1f, 1000.0f);
	}

	float Camera::getWeight() const {
		return impl->weight;
	}

	void Camera::setWeight(float newWeight) {
		impl->weight = newWeight;
	}


	glm::vec3 Yngin::BlendedCamera::getPosition() const {
		glm::vec3 finalPos = {};
		for (auto& camera : impl->mgr->getCameras()) {
			// pos * weight
			finalPos += camera->impl->pos * camera->impl->weight;
		}
		finalPos /= impl->mgr->getTotalWeight();
		return finalPos;
	}

	glm::vec3 Yngin::BlendedCamera::getOrientation() const {
		glm::vec3 finalOrientation = {};
		for (auto& camera : impl->mgr->getCameras()) {
			// orientation * weight
			finalOrientation += camera->impl->orientation * camera->impl->weight;
		}
		finalOrientation /= impl->mgr->getTotalWeight();
		return finalOrientation;
	}

	float Yngin::BlendedCamera::getFov() const {
		float finalFov = 0.0f;
		for (auto& camera : impl->mgr->getCameras()) {
			// fov * weight
			finalFov += camera->impl->fov * camera->impl->weight;
		}
		finalFov /= impl->mgr->getTotalWeight();
		return finalFov;
	}
}
