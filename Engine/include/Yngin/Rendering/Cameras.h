#pragma once
#include <Yngin/Forward.h>
#include <memory>
#include <glm/vec3.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "../Services/Tween.h"
#include <Yngin/Utils/Meta.h>

namespace Yngin {
	class Camera {
	public:
		uint32_t getId() const;
		Context* getContext() const;
		Scene* getScene() const;

		Meta meta;

		virtual glm::vec3 getPosition() const;
		virtual glm::vec3 getOrientation() const;
		virtual float getFov() const;

		void setPosition(glm::vec3 newPos);
		void setOrientation(glm::vec3 newOrientation);
		void setFov(float newFov);

		void lookAt(glm::vec3 target);

		float getWeight() const;
		void setWeight(float newWeight);

	private:
		friend class CamerasManager;
		friend struct std::default_delete<Camera>;
		friend class Services::Tween;
		friend class BlendedCamera;
		friend class Rendering::Renderer;

		Camera(Context* ctx, Scene* scene);
		~Camera();

		struct Impl;
		std::unique_ptr<Impl> impl;
	};

	class CamerasManager {
	public:
		Context* getContext() const;

		Camera* createCamera();
		Camera* createCamera(uint32_t id, bool override = false);
		void deleteCamera(uint32_t cameraId);
		void deleteCamera(Camera* camera);

		size_t getCamerasCount() const;
		std::vector<Camera*> getCameras() const;
		Camera* getCamera(uint32_t cameraId) const;

		Camera* getBlendedCamera() const;

		float getTotalWeight() const;

		// sets all cameras' weights to zero and this cameraId to 1
		void setActive(uint32_t cameraId);
		void setActive(Camera* camera);

	private:
		friend class Scene;
		friend struct std::default_delete<CamerasManager>;
		friend class Rendering::Renderer;
		friend class BlendedCamera;

		CamerasManager(Scene* scene);
		~CamerasManager();

		struct Impl;
		std::unique_ptr<Impl> impl;
	};
}
