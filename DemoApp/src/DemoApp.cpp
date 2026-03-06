// I know this code isn't the best but I'll improve it soon
//
// I just quickly add code to test new features without
// checking the code style

#include <stdio.h>
#include <fstream>
#include <sstream>
#include <Yngin/Yngin.h>
#include <glm/glm.hpp>
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

using namespace Yngin;

int main() {
	Yngin::init();

	ContextSettings settings = {};
	settings.windowSettings = {
		.title = "Yngin Demo App"
	};

	Context* ctx = createContext(settings);

	printf("Context: %p\n", ctx);

	ModelsManager* modelsMgr = ctx->getModelsManager();
	ScenesManager* scenesManager = ctx->getScenesManager();
	Scene* scene = scenesManager->createScene();
	GameObjectsManager* gameObjMgr = scene->getGameObjectsManager();

	Window* window = ctx->getWindow();
	InputSystem* input = ctx->getInputSystem();

	scenesManager->setActive(scene->getId());

	std::ifstream modelFile("test_model");

	if (!modelFile.is_open()) {
		printf("Test model not found\n");
		Yngin::terminate();
		return 1;
	}

	std::stringstream modelFileData;
	modelFileData << modelFile.rdbuf();

	modelFile.close();

	Model* model = modelsMgr->createModel(MODEL_FILE_TYPE::OBJ, modelFileData.str().c_str(), modelFileData.str().length());

	CamerasManager* camerasManager = scene->getCamerasManager();

	Camera* defaultCamera = camerasManager->getCamera(0);

	defaultCamera->setPos({ 10, 0, 0 });
	defaultCamera->lookAt(glm::vec3());

	TexturesManager* texMgr = ctx->getTexturesManager();

	// https://freestylized.com/skybox/sky_36/
	Texture* skyboxTex = texMgr->createTexture();
	{
		TextureData skyboxData{};
		unsigned char* bytes = stbi_load("skybox.png", &skyboxData.width, &skyboxData.height, &skyboxData.numCh, 0);
		skyboxData.data = (const char*)bytes;
		skyboxData.wrap = TEXTURE_WRAP::CLAMP;
		skyboxData.filter = TEXTURE_FILTER::NEAREST;
		if (skyboxData.data) {
			skyboxTex->setData(skyboxData);
			stbi_image_free(bytes);
		} else {
			printf("Failed to load skybox: %s\n", stbi_failure_reason());
		}
	}

	scene->setSkyboxTexture(skyboxTex);

	Texture* githubTex = texMgr->createTexture();
	{
		TextureData data{};
		unsigned char* bytes = stbi_load("github.png", &data.width, &data.height, &data.numCh, 0);
		data.data = (const char*)bytes;
		data.wrap = TEXTURE_WRAP::CLAMP;
		data.filter = TEXTURE_FILTER::LINEAR;
		if (data.data) {
			githubTex->setData(data);
			stbi_image_free(bytes);
		} else {
			printf("Failed to load github.png: %s\n", stbi_failure_reason());
		}
	}

	TextureData texData{};
	texData.width = 2;
	texData.height = 2;
	texData.numCh = 1;
	texData.wrap = TEXTURE_WRAP::REPEAT;
	texData.filter = TEXTURE_FILTER::NEAREST;
	texData.data = "\xff\x80\x80\xff";
	Texture* tex = texMgr->createTexture(texData);

	TextureData whiteTexData = {
		.width = 1,
		.height = 1,
		.numCh = 1,
		.data = "\xaa"
	};
	Texture* whiteTex = texMgr->createTexture(whiteTexData);

	GameObject* obj = gameObjMgr->getRootGameObject()->createChild();
	Components::Mesh* mesh = obj->createComponent<Components::Mesh>();
	obj->setRotation({ glm::radians(90.0f), 0, 0 });

	mesh->setModel(model->getId());
	mesh->setTexture(tex->getId());

	std::vector<Vertex> wallVertices;
	std::vector<uint32_t> wallIndices = { 0, 1, 2, 0, 2, 3 };

	wallVertices.push_back({ glm::vec3(+0.5f, +0.5f, 0.0f), glm::vec2(100.0f, 000.0f) });
	wallVertices.push_back({ glm::vec3(-0.5f, +0.5f, 0.0f), glm::vec2(000.0f, 000.0f) });
	wallVertices.push_back({ glm::vec3(-0.5f, -0.5f, 0.0f), glm::vec2(000.0f, 100.0f) });
	wallVertices.push_back({ glm::vec3(+0.5f, -0.5f, 0.0f), glm::vec2(100.0f, 100.0f) });

	ModelData wallModelData = { wallVertices, wallIndices };
	Model* wallModel = modelsMgr->createModel(wallModelData);

	GameObject* wall = gameObjMgr->getRootGameObject()->createChild();
	Components::Mesh* wallMesh = wall->createComponent<Components::Mesh>();
	wallMesh->setModel(wallModel->getId());
	wallMesh->setTexture(tex->getId());
	wallMesh->setScale(glm::vec3(1, 1, 0) * 100.0f);
	wall->setPos({ 0, 0, -5.0f });

	ctx->setMaxFPS(120);

	Services::Tween* tween = ctx->getService<Services::Tween>();

	Services::TweenSettings tweenSettings = {
		.duration = 1.0f,
		.function = Services::TWEEN_FUNCTION::EASE_INOUT
	};

	int tweenId = 0;
	int cycle = 0;

	UI::Image* github = scene->getUIManager()->getRootElement()->createChild<UI::Image>();
	github->setTexture(githubTex->getId());
	github->setPos({ 0, 48, 1.0f, -48 });
	github->setSize({ 0, 64, 0, 64 });

	glm::ivec2 oldMousePos = {};

	while (ctx->getStatus() == CONTEXT_STATUS::RUNNING) {
		ctx->makeCurrent();

		window->setCursorLocked(input->isMousePressed(MOUSE_BUTTON::RIGHT));

		if (!tween->isActive(tweenId)) {
			cycle = (cycle + 1) % 3;
			tweenId = tween->tweenPos(obj, glm::vec3(0, cycle == 1 ? 1.0f : cycle == 2 ? -1.0f : 0, cycle == 0 ? -1.0f : 1.0f) * 2.0f, tweenSettings);
		}

		if (input->isKeyJustPressed(Yngin::KEY::SPACE)) {
			tween->setPaused(tweenId, !tween->isPaused(tweenId));
		}

		if (github->isClicked()) {
			system("start https://github.com/YusufYaser/Yngin");
		}

		if (github->isHeld()) {
			github->setColor(glm::vec4(0.5f));
		} else if (github->isHovered()) {
			github->setColor(glm::vec4(0.75f));
		} else {
			github->setColor(glm::vec4(1.0f));
		}

		double delta = ctx->getDeltaTime();

		if (input->isMouseJustPressed(MOUSE_BUTTON::RIGHT)) {
			oldMousePos = input->getMousePos();
		}

		if (input->isMousePressed(MOUSE_BUTTON::RIGHT)) {
			glm::vec3 o = defaultCamera->getOrientation();
			glm::ivec2 m = input->getMousePos() - oldMousePos;
			oldMousePos = input->getMousePos();

			float senstivity = 0.002f;

			float yaw = atan2(o.x, o.y);
			float pitch = asin(o.z);

			o.x = cos(pitch) * sin(yaw + m.x * senstivity);
			o.y = cos(pitch) * cos(yaw + m.x * senstivity);
			o.z = sin(pitch - m.y * senstivity);

			defaultCamera->setOrientation(o);

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

			defaultCamera->setPos(defaultCamera->getPos() + change * (float)delta * speed);
		}

		ctx->update();
	}

	Yngin::terminate();

	return 0;
}
