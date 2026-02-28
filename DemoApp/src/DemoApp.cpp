#include <stdio.h>
#include <fstream>
#include <sstream>
#include <Yngin/Yngin.h>
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

	ScenesManager* scenesManager = ctx->getScenesManager();
	Scene* scene = scenesManager->createScene();

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

	Model* model = ctx->getModelsManager()->createModel(MODEL_FILE_TYPE::OBJ, modelFileData.str().c_str(), modelFileData.str().length());

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

	GameObject* obj = scene->getGameObjectsManager()->getRootGameObject()->createChild();
	Components::Mesh* mesh = obj->createComponent<Components::Mesh>();
	obj->setRotation({ glm::radians(90.0f), 0, 0 });

	mesh->setModel(model->getId());
	mesh->setTexture(tex->getId());

	ctx->setMaxFPS(120);

	while (ctx->getStatus() == CONTEXT_STATUS::RUNNING) {
		ctx->makeCurrent();

		double time = ctx->getTime();
		obj->setRotation(glm::vec3(time, time * 2, time * 3));

		ctx->update();
	}

	Yngin::terminate();

	return 0;
}
