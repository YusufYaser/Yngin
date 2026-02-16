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

	ctx->getScenesManager()->getCamerasManager(0)->setPos(0, { 5, 5, 5 });
	ctx->getScenesManager()->getCamerasManager(0)->lookAt(0, { 0, 0, 0 });

	while (!ctx->windowShouldClose()) {
		ctx->updateWindow();
		ctx->getScenesManager()->render(scene);
		ctx->swapBuffers();
	}

	Yngin::terminate();

	return 0;
}
