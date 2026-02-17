#include <stdio.h>
#include <Yngin/Yngin.h>
#include <Yngin/Scenes.h>
#include <Yngin/Models.h>
#include <Yngin/Cameras.h>

using namespace Yngin;

int main() {
	Yngin::init();

	Context* ctx = createContext();

	printf("Context: %p\n", ctx);

	uint32_t scene = ctx->getScenesManager()->createScene();

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

	CamerasManager* camerasManager = ctx->getScenesManager()->getCamerasManager(0);

	Camera* defaultCamera = camerasManager->getCamera(0);
	defaultCamera->setPos({ 5, 5, 5 });
	defaultCamera->lookAt({ 0, 0, 0 });

	uint32_t newCameraId = camerasManager->createCamera();
	Camera* newCamera = camerasManager->getCamera(newCameraId);

	newCamera->setPos({ -4, 5, 5 });
	newCamera->lookAt({ 0, 0, 0 });

	uint64_t frameNum = 0;
	while (!ctx->windowShouldClose()) {
		ctx->updateWindow();

		defaultCamera->setWeight((frameNum % 1000) / 1000.0f);
		newCamera->setWeight(1 - (frameNum % 1000) / 1000.0f);

		ctx->getScenesManager()->render(scene);
		ctx->swapBuffers();

		frameNum++;
	}

	Yngin::terminate();

	return 0;
}
