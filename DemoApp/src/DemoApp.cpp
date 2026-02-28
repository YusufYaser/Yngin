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

	defaultCamera->setPos({ 5, 5, 0 });
	defaultCamera->lookAt(glm::vec3());

	TexturesManager* texMgr = ctx->getTexturesManager();

	// https://freestylized.com/skybox/sky_36/
	Texture* skyboxTex = texMgr->createTexture();
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

	scene->setSkyboxTexture(skyboxTex);

	TextureData texData{};
	texData.width = 2;
	texData.height = 2;
	texData.numCh = 1;
	texData.wrap = TEXTURE_WRAP::CLAMP;
	texData.filter = TEXTURE_FILTER::NEAREST;
	texData.data = "\xff\x00\x00\xff";
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

	wallVertices.push_back({ glm::vec3(0.5f, -0.5f, 0), glm::vec2(0, 0), glm::vec3(0, 0, 1) });
	wallVertices.push_back({ glm::vec3(-0.5f, -0.5f, 0), glm::vec2(0, 0), glm::vec3(0, 0, 1) });
	wallVertices.push_back({ glm::vec3(-0.5f,  0.5f, 0), glm::vec2(0, 0), glm::vec3(0, 0, 1) });
	wallVertices.push_back({ glm::vec3(0.5f,  0.5f, 0), glm::vec2(0, 0), glm::vec3(0, 0, 1) });

	ModelData wallModelData = { wallVertices, wallIndices };
	Model* wallModel = modelsMgr->createModel(wallModelData);

	GameObject* wall = gameObjMgr->getRootGameObject()->createChild();
	Components::Mesh* wallMesh = wall->createComponent<Components::Mesh>();
	wallMesh->setModel(wallModel->getId());
	wallMesh->setTexture(whiteTex->getId());
	wallMesh->setScale(glm::vec3(1, 1, 0) * 100.0f);
	wall->setPos({ 0, 0, -1.0f });

	ctx->setMaxFPS(120);

	while (ctx->getStatus() == CONTEXT_STATUS::RUNNING) {
		ctx->makeCurrent();

		window->setCursorLocked(input->isMousePressed(MOUSE_BUTTON::RIGHT));

		double time = ctx->getTime();
		obj->setRotation(glm::vec3(3.14f / 2, time * 0.25f, 0));
		obj->setPos(glm::vec3(0, 0, sin(time * 1.5f) * 0.25f) / 2.0f + 0.5f);

		ctx->update();
	}

	Yngin::terminate();

	return 0;
}
