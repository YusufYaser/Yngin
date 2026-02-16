#pragma once
#include "../Scenes/Scenes_Internal.h"
#include <glm/vec3.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace Yngin {
	class Camera {
	public:
		Camera(Context* ctx, Scene* scene);
		~Camera();

		glm::vec3 getPos() const { return pos; };
		glm::vec3 getOrientation() const { return orientation; };
		float getFov() const { return fov; };

		void setPos(glm::vec3 newPos) { pos = newPos; };
		void setOrientation(glm::vec3 newOrientation) { orientation = newOrientation; };
		void setFov(float newFov) { fov = newFov; };

		void lookAt(glm::vec3 target);

		glm::mat4 getView();
		glm::mat4 getProjection();

	private:
		Context* ctx;
		Scene* scene;

		float fov = 70.0f;
		glm::vec3 pos = { 0, 0, 0 };
		glm::vec3 orientation = { 1, 0, 0 };
	};
}
