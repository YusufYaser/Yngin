#include <stdio.h>
#include <Yngin/Core/Context.h>
#include <Yngin/Core/Scenes.h>
#include <Yngin/Rendering/Cameras.h>
#include <fstream>
#include <sstream>

#ifdef _WIN32
#include <windows.h>
#endif

using namespace Yngin;

void handleCameraMovement(Yngin::Camera* camera);

void error(const char* text) {
#ifdef _WIN32
	MessageBoxA(NULL, text, "Yngin Runtime", MB_ICONERROR | MB_OK);
#else
	printf(text);
#endif
}

int main() {
	Yngin::initializeYngin();

	if (!Yngin::isYnginInitialized()) {
		error("Failed to initialize Yngin\n");
	}

	std::ifstream scenePak("scene.pak", std::ios::binary);
	if (!scenePak.is_open()) {
		error("Failed to open scene.pak");
		Yngin::terminateYngin();
		return 1;
	}

	std::ifstream resourcesPak("resources.pak", std::ios::binary);
	if (!scenePak.is_open()) {
		error("Failed to open resources.pak");
		scenePak.close();
		Yngin::terminateYngin();
		return 1;
	}

	Context* ctx = new Context();

	std::ostringstream resourcesBytes(std::ios::binary);
	resourcesBytes << resourcesPak.rdbuf();
	resourcesPak.close();
	ctx->loadResourcesPak(resourcesBytes.str().c_str(), resourcesBytes.str().size());
	resourcesBytes.clear();

	std::ostringstream sceneBytes(std::ios::binary);
	sceneBytes << scenePak.rdbuf();
	scenePak.close();
	Scene* scene = ctx->getScenesManager()->createScene(sceneBytes.str().c_str(), sceneBytes.str().size(), 0, true);
	sceneBytes.clear();

	scene->activate();

	Camera* camera = scene->getCamerasManager()->getCamera(0);

	while (ctx->getStatus() == CONTEXT_STATUS::RUNNING) {
		handleCameraMovement(camera);
		ctx->update();
	}

	Yngin::terminateYngin();

	return 0;
}

#ifdef _WIN32
int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR pCmdLine, int nCmdShow) {
	return main();
}
#endif
