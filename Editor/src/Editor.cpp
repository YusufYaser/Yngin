#include <stdio.h>
#include <Yngin/Yngin.h>
#include <ImGui/imgui.h>
#include <ImGui/imgui_impl_opengl3.h>
#include <ImGui/imgui_impl_glfw.h>

using namespace Yngin;

void handleCameraMovement(Camera* camera) {
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

int main() {
	initializeYngin();

	if (!isYnginInitialized()) {
		printf("Failed to initialize Yngin\n");
		return 1;
	}

	Context* ctx = createContext();

	if (ctx == nullptr || ctx->getStatus() != CONTEXT_STATUS::RUNNING) {
		printf("Failed to create context\n");
		return 1;
	}

	Scene* scene = ctx->getScenesManager()->createScene();
	scene->activate();

	TexturesManager* texturesManager = ctx->getTexturesManager();

	Texture* skyboxTex = ctx->getTexturesManager()->createTexture("assets/default_skybox.png", {
		.wrap = TEXTURE_WRAP::CLAMP,
		.filterMin = TEXTURE_FILTER::NEAREST,
		.filterMag = TEXTURE_FILTER::NEAREST,
		});
	scene->setSkyboxTexture(skyboxTex);

	InputSystem* input = ctx->getInputSystem();
	Camera* editorCamera = scene->getCamerasManager()->getCamera(0);

	while (ctx->getStatus() == CONTEXT_STATUS::RUNNING) {
		ctx->update(false);

		handleCameraMovement(editorCamera);

		ctx->swapBuffers();
	}

	delete ctx;
	ctx = nullptr;

	terminateYngin();

	return 0;
}
