#pragma once
#include <map>
#include <Yngin/Core/Context.h>

namespace Yngin {
	class BlendedCamera : public Camera {
	public:
		glm::vec3 getPosition() const override;
		glm::vec3 getOrientation() const override;
		float getFov() const override;

	private:
		friend class CamerasManager;
		friend struct std::default_delete<BlendedCamera>;

		BlendedCamera(Context* ctx, Scene* scene) : Camera(ctx, scene) {};
		~BlendedCamera() = default;
	};

	struct Camera::Impl {
		Context* ctx;
		Scene* scene;
		CamerasManager* mgr;
		Camera* owner;

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

		std::unique_ptr<BlendedCamera> blendedCamera;

		std::map<uint32_t, std::unique_ptr<Camera>> cameras;
		uint32_t nextCameraId = 0;
	};
}
