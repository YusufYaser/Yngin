#include <stdio.h>
#include <Yngin/Yngin.h>
#include <Yngin/Scenes.h>
#include <Yngin/Models.h>
#include <Yngin/Cameras.h>
#include <Yngin/GameObject.h>
#include <Yngin/Textures.h>

using namespace Yngin;

int main() {
	Yngin::init();

	Context* ctx = createContext();

	printf("Context: %p\n", ctx);

	uint32_t sceneId = ctx->getScenesManager()->createScene();
	Scene* scene = ctx->getScenesManager()->getScene(sceneId);

	std::vector<Vertex> cubeVertices;
	cubeVertices.push_back(Vertex{ { 0.5f, -0.5f, 0.5f, } });
	cubeVertices.push_back(Vertex{ { -0.5f, -0.5f, 0.5f, } });
	cubeVertices.push_back(Vertex{ { -0.5f, 0.5f, 0.5f, } });
	cubeVertices.push_back(Vertex{ { 0.5f, 0.5f, 0.5f, } });
	cubeVertices.push_back(Vertex{ { -0.5f, -0.5f, -0.5f, } });
	cubeVertices.push_back(Vertex{ { 0.5f, -0.5f, -0.5f, } });
	cubeVertices.push_back(Vertex{ { 0.5f, 0.5f, -0.5f, } });
	cubeVertices.push_back(Vertex{ { -0.5f, 0.5f, -0.5f, } });
	cubeVertices.push_back(Vertex{ { 0.5f, -0.5f, -0.5f, } });
	cubeVertices.push_back(Vertex{ { 0.5f, -0.5f, 0.5f, } });
	cubeVertices.push_back(Vertex{ { 0.5f, 0.5f, 0.5f, } });
	cubeVertices.push_back(Vertex{ { 0.5f, 0.5f, -0.5f, } });
	cubeVertices.push_back(Vertex{ { -0.5f, -0.5f, 0.5f, } });
	cubeVertices.push_back(Vertex{ { -0.5f, -0.5f, -0.5f, } });
	cubeVertices.push_back(Vertex{ { -0.5f, 0.5f, -0.5f, } });
	cubeVertices.push_back(Vertex{ { -0.5f, 0.5f, 0.5f, } });
	cubeVertices.push_back(Vertex{ { -0.5f, -0.5f, 0.5f, } });
	cubeVertices.push_back(Vertex{ { 0.5f, -0.5f, 0.5f, } });
	cubeVertices.push_back(Vertex{ { 0.5f, -0.5f, -0.5f, } });
	cubeVertices.push_back(Vertex{ { -0.5f, -0.5f, -0.5f, } });
	cubeVertices.push_back(Vertex{ { 0.5f, 0.5f, 0.5f, } });
	cubeVertices.push_back(Vertex{ { -0.5f, 0.5f, 0.5f, } });
	cubeVertices.push_back(Vertex{ { -0.5f, 0.5f, -0.5f, } });
	cubeVertices.push_back(Vertex{ { 0.5f, 0.5f, -0.5 } });

	std::vector<uint32_t> cubeIndices = {
		0, 1, 2, 0, 2, 3,
		4, 5, 6, 4, 6, 7,
		8, 9, 10, 8, 10, 11,
		12, 13, 14, 12, 14, 15,
		16, 17, 18, 16, 18, 19,
		20, 21, 22, 20, 22, 23
	};

	uint32_t model = ctx->getModelsManager()->createModel(cubeVertices, cubeIndices);

	uint32_t objId = scene->getGameObjectsManager()->getRootGameObject()->createChild();
	uint32_t obj2Id = scene->getGameObjectsManager()->getRootGameObject()->createChild();
	uint32_t obj3Id = scene->getGameObjectsManager()->getRootGameObject()->createChild();
	scene->getGameObjectsManager()->getGameObject(obj2Id)->setPos({ 0, 0, 2 });
	scene->getGameObjectsManager()->getGameObject(obj3Id)->setPos({ 0, 1, 1 });

	CamerasManager* camerasManager = scene->getCamerasManager();

	Camera* defaultCamera = camerasManager->getCamera(0);
	defaultCamera->setPos({ 5, 5, 5 });
	defaultCamera->lookAt({ 0, 0, 0 });

	uint32_t newCameraId = camerasManager->createCamera();
	Camera* newCamera = camerasManager->getCamera(newCameraId);

	newCamera->setPos({ -4, 5, 5 });
	newCamera->lookAt({ 0, 0, 0 });

	ctx->getTexturesManager()->createTexture(2, 1, 2, "\xff\xff\x0f\xff");

	uint64_t frameNum = 0;
	while (!ctx->windowShouldClose()) {
		ctx->updateWindow();

		defaultCamera->setWeight((frameNum % 1000) / 1000.0f);
		newCamera->setWeight(1 - (frameNum % 1000) / 1000.0f);

		scene->render();
		ctx->swapBuffers();

		frameNum++;
	}

	Yngin::terminate();

	return 0;
}
