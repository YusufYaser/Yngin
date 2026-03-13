#include <Yngin/Core/Context.h>
#include <Yngin/Core/InputSystem.h>
#include "Editor.h"

using namespace Yngin;

void Editor::handleCameraMovement(Yngin::Camera* camera) {
	Context* ctx = camera->getContext();
	InputSystem* input = ctx->getInputSystem();

	double delta = ctx->getDeltaTime();

	ctx->getWindow()->setCursorLocked(input->isMousePressed(MOUSE_BUTTON::RIGHT));

	if (input->isMouseJustPressed(MOUSE_BUTTON::RIGHT)) {
		input->setMousePosition({ 0, 0 });
	}

	if (input->isMousePressed(MOUSE_BUTTON::RIGHT)) {
		glm::vec3 o = camera->getOrientation();
		glm::ivec2 m = input->getMousePosition(true);
		input->setMousePosition({ 0, 0 });

		float senstivity = 0.002f;

		float yaw = atan2(o.x, o.y);
		float pitch = asin(o.z);

		o.x = cos(pitch) * sin(yaw + m.x * senstivity);
		o.y = cos(pitch) * cos(yaw + m.x * senstivity);
		o.z = sin(pitch - m.y * senstivity);

		camera->setOrientation(o);

		glm::vec3 forward = glm::normalize(o);
		glm::vec3 right = glm::normalize(glm::cross(forward, glm::vec3(0, 0, 1)));
		glm::vec3 realUp = glm::cross(forward, right);

		glm::vec3 change = {};

		if (input->isKeyPressed(Yngin::KEY::W)) {
			change += forward;
		}
		if (input->isKeyPressed(Yngin::KEY::S)) {
			change -= forward;
		}
		if (input->isKeyPressed(Yngin::KEY::D)) {
			change += right;
		}
		if (input->isKeyPressed(Yngin::KEY::A)) {
			change -= right;
		}
		if (input->isKeyPressed(Yngin::KEY::Q)) {
			change += realUp;
		}
		if (input->isKeyPressed(Yngin::KEY::E)) {
			change -= realUp;
		}

		float speed = 5.0f;
		if (input->isKeyPressed(Yngin::KEY::LSHIFT)) {
			speed *= 1.5f;
		}

		camera->setPosition(camera->getPosition() + change * (float)delta * speed);
	}
}
