#include <Yngin/Rendering/Cameras.h>
#include "Cameras_Internal.h"
#include "../../Core/Scenes/Scenes_Internal.h"
#include <assert.h>
#include <stdexcept>

namespace Yngin {
	CamerasManager::CamerasManager(Scene* scene) {
		impl = std::make_unique<Impl>();

		impl->ctx = scene->getContext();
		impl->scene = scene;
		impl->owner = this;

		Camera* defaultCamera = createCamera(0, true);
		defaultCamera->setWeight(1.0f);

		impl->blendedCamera = std::unique_ptr<BlendedCamera>(new BlendedCamera(impl->ctx, impl->scene));
		impl->blendedCamera->impl->mgr = this;
	}

	CamerasManager::~CamerasManager() = default;

	Context* CamerasManager::getContext() const {
		return impl->ctx;
	}

	Camera* CamerasManager::createCamera() {
		return createCamera(impl->nextCameraId);
	}

	Camera* CamerasManager::createCamera(uint32_t id, bool override) {
		if (getCamera(id) != nullptr) {
			if (override) {
				impl->cameras.erase(id);
			} else {
				return nullptr;
			}
		}

		auto camera = std::unique_ptr<Camera>(new Camera(impl->ctx, impl->scene));

		int nextId = impl->nextCameraId;
		impl->nextCameraId = std::max(impl->nextCameraId, id + 1);
		camera->impl->id = id;
		camera->impl->mgr = this;
		impl->cameras[id] = std::move(camera);
		return impl->cameras[id].get();
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

	size_t CamerasManager::getCamerasCount() const {
		return impl->cameras.size();
	}

	std::vector<Camera*> CamerasManager::getCameras() const {
		std::vector<Camera*> cameras;
		for (auto& kvp : impl->cameras) {
			cameras.push_back(kvp.second.get());
		}
		return cameras;
	}

	Camera* CamerasManager::getCamera(uint32_t cameraId) const {
		auto it = impl->cameras.find(cameraId);
		if (it == impl->cameras.end()) return nullptr;

		return it->second.get();
	}

	Camera* CamerasManager::getBlendedCamera() const {
		return impl->blendedCamera.get();
	}

	float CamerasManager::getTotalWeight() const {
		float totalWeight = 0.0f;

		for (auto& kvp : impl->cameras) {
			totalWeight += kvp.second->impl->weight;
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
}
