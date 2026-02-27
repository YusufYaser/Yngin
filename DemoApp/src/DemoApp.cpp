#include <stdio.h>
#include <fstream>
#include <sstream>
#include <Yngin/Yngin.h>

using namespace Yngin;

int main() {
	Yngin::init();

	Context* ctx = createContext();

	printf("Context: %p\n", ctx);

	ScenesManager* scenesManager = ctx->getScenesManager();
	uint32_t sceneId = scenesManager->createScene();
	Scene* scene = scenesManager->getScene(sceneId);

	scenesManager->setActive(sceneId);

	std::ifstream modelFile("test_model.obj");

	if (!modelFile.is_open()) {
		printf("Test model not found\n");
		Yngin::terminate();
		return 1;
	}

	std::stringstream modelFileData;
	modelFileData << modelFile.rdbuf();

	modelFile.close();

	uint32_t model = ctx->getModelsManager()->createModel(MODEL_FILE_TYPE::OBJ, modelFileData.str().c_str(), modelFileData.str().length());

	uint32_t objId = scene->getGameObjectsManager()->getRootGameObject()->createChild();

	CamerasManager* camerasManager = scene->getCamerasManager();

	Camera* defaultCamera = camerasManager->getCamera(0);

	defaultCamera->setPos(glm::vec3(5.0f));
	defaultCamera->lookAt(glm::vec3());

	TextureData texData{};
	texData.width = 2;
	texData.height = 1;
	texData.numCh = 2;
	texData.wrap = TEXTURE_WRAP::CLAMP;
	texData.data = "\xff\xff\x0f\xff";
	uint32_t texId = ctx->getTexturesManager()->createTexture(texData);
	Texture* tex = ctx->getTexturesManager()->getTexture(texId);

	GameObject* obj = scene->getGameObjectsManager()->getGameObject(objId);
	Components::Mesh* mesh = obj->createComponent<Components::Mesh>();
	obj->setRotation({ glm::radians(90.0f), 0, 0 });

	mesh->setModel(model);
	mesh->setTexture(texId);

	ctx->setMaxFPS(120);

	while (!ctx->isClosing()) {
		ctx->makeCurrent();

		double time = ctx->getTime();
		obj->setRotation(glm::vec3(time, time * 2, time * 3));

		ctx->update();
	}

	Yngin::terminate();

	return 0;
}
