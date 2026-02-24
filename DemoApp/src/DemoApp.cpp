#include <stdio.h>
#include <fstream>
#include <sstream>
#include <Yngin/Yngin.h>
#include <Yngin/Window.h>
#include <Yngin/Scenes.h>
#include <Yngin/Models.h>
#include <Yngin/Cameras.h>
#include <Yngin/GameObject.h>
#include <Yngin/Textures.h>
#include <Yngin/Components/Mesh.h>

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

	while (!ctx->isClosing()) {
		uint64_t frameNum = ctx->getFrame();

		defaultCamera->setPos(glm::vec3(sin(frameNum / 1000.0), cos(frameNum / 1000.0), 1) * 5.0f);
		defaultCamera->lookAt(glm::vec3());

		ctx->update();
	}

	Yngin::terminate();

	return 0;
}
