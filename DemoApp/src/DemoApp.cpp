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

	uint32_t newCameraId = camerasManager->createCamera();
	Camera* newCamera = camerasManager->getCamera(newCameraId);

	newCamera->setPos({ -4, 5, 5 });
	newCamera->lookAt({ 0, 0, 0 });

	ctx->getTexturesManager()->createTexture(2, 1, 2, "\xff\xff\x0f\xff");

	GameObject* obj = scene->getGameObjectsManager()->getGameObject(objId);
	obj->createComponent<Components::Mesh>();
	obj->setRotation({ glm::radians(90.0f), 0, 0 });

	ctx->setMaxFPS(120);

	while (!ctx->isClosing()) {
		ctx->makeCurrent();

		defaultCamera->setPos(glm::vec3(sin(ctx->getTime()), cos(ctx->getTime()), 1) * 5.0f);
		defaultCamera->lookAt(glm::vec3());

		ctx->update();
	}

	Yngin::terminate();

	return 0;
}
