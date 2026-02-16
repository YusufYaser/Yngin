#include <stdio.h>
#include <Yngin/Yngin.h>
#include <Yngin/Scenes.h>
#include <Yngin/Models.h>
#include <Yngin/Cameras.h>

int main() {
	Yngin::init();

	Yngin::Context* ctx = Yngin::createContext();

	printf("Context: %p\n", ctx);

	uint32_t scene = ctx->getScenesManager()->createScene();

	std::vector<Yngin::Vertex> cubeVertices;
	cubeVertices.push_back(Yngin::Vertex{ { 0.5f, -0.5f, 0.5f, } });
	cubeVertices.push_back(Yngin::Vertex{ { -0.5f, -0.5f, 0.5f, } });
	cubeVertices.push_back(Yngin::Vertex{ { -0.5f, 0.5f, 0.5f, } });
	cubeVertices.push_back(Yngin::Vertex{ { 0.5f, 0.5f, 0.5f, } });
	cubeVertices.push_back(Yngin::Vertex{ { -0.5f, -0.5f, -0.5f, } });
	cubeVertices.push_back(Yngin::Vertex{ { 0.5f, -0.5f, -0.5f, } });
	cubeVertices.push_back(Yngin::Vertex{ { 0.5f, 0.5f, -0.5f, } });
	cubeVertices.push_back(Yngin::Vertex{ { -0.5f, 0.5f, -0.5f, } });
	cubeVertices.push_back(Yngin::Vertex{ { 0.5f, -0.5f, -0.5f, } });
	cubeVertices.push_back(Yngin::Vertex{ { 0.5f, -0.5f, 0.5f, } });
	cubeVertices.push_back(Yngin::Vertex{ { 0.5f, 0.5f, 0.5f, } });
	cubeVertices.push_back(Yngin::Vertex{ { 0.5f, 0.5f, -0.5f, } });
	cubeVertices.push_back(Yngin::Vertex{ { -0.5f, -0.5f, 0.5f, } });
	cubeVertices.push_back(Yngin::Vertex{ { -0.5f, -0.5f, -0.5f, } });
	cubeVertices.push_back(Yngin::Vertex{ { -0.5f, 0.5f, -0.5f, } });
	cubeVertices.push_back(Yngin::Vertex{ { -0.5f, 0.5f, 0.5f, } });
	cubeVertices.push_back(Yngin::Vertex{ { -0.5f, -0.5f, 0.5f, } });
	cubeVertices.push_back(Yngin::Vertex{ { 0.5f, -0.5f, 0.5f, } });
	cubeVertices.push_back(Yngin::Vertex{ { 0.5f, -0.5f, -0.5f, } });
	cubeVertices.push_back(Yngin::Vertex{ { -0.5f, -0.5f, -0.5f, } });
	cubeVertices.push_back(Yngin::Vertex{ { 0.5f, 0.5f, 0.5f, } });
	cubeVertices.push_back(Yngin::Vertex{ { -0.5f, 0.5f, 0.5f, } });
	cubeVertices.push_back(Yngin::Vertex{ { -0.5f, 0.5f, -0.5f, } });
	cubeVertices.push_back(Yngin::Vertex{ { 0.5f, 0.5f, -0.5 } });

	std::vector<uint32_t> cubeIndices = {
		0, 1, 2, 0, 2, 3,
		4, 5, 6, 4, 6, 7,
		8, 9, 10, 8, 10, 11,
		12, 13, 14, 12, 14, 15,
		16, 17, 18, 16, 18, 19,
		20, 21, 22, 20, 22, 23
	};

	uint32_t model = ctx->getModelsManager()->createModel(cubeVertices, cubeIndices);

	Yngin::CamerasManager* camerasManager = ctx->getScenesManager()->getCamerasManager(0);
	camerasManager->setPos(0, { 5, 5, 5 });
	camerasManager->lookAt(0, { 0, 0, 0 });

	uint32_t newCamera = camerasManager->createCamera();
	camerasManager->setPos(newCamera, { -4, 5, 5 });
	camerasManager->lookAt(newCamera, { 0, 0, 0 });

	uint64_t frameNum = 0;
	while (!ctx->windowShouldClose()) {
		ctx->updateWindow();

		camerasManager->setWeight(0, (frameNum % 1000) / 1000.0f);
		camerasManager->setWeight(1, 1 - (frameNum % 1000) / 1000.0f);

		ctx->getScenesManager()->render(scene);
		ctx->swapBuffers();

		frameNum++;
	}

	Yngin::terminate();

	return 0;
}
