#include <stdio.h>
#include <Yngin/Core/Context.h>
#include <Yngin/Core/Scenes.h>
#include <Yngin/Core/InputSystem.h>
#include <Yngin/Rendering/Cameras.h>
#include <fstream>
#include <sstream>

#ifdef _WIN32
#include <windows.h>
#endif

using namespace Yngin;

void error(const char* text) {
	printf("%s\n", text);
#ifdef _WIN32
	MessageBoxA(NULL, text, "Yngin Runtime", MB_ICONERROR | MB_OK);
#endif
}

int main() {
#ifdef _WIN32
	bool consoleShown = false;

	if (AllocConsole()) {
		FILE* fDummy;
		freopen_s(&fDummy, "CONOUT$", "w", stdout);
		freopen_s(&fDummy, "CONOUT$", "w", stderr);
		freopen_s(&fDummy, "CONIN$", "r", stdin);
	}

	ShowWindow(GetConsoleWindow(), SW_HIDE);
#endif

	Yngin::initializeYngin();

	if (!Yngin::isYnginInitialized()) {
		error("Failed to initialize Yngin");
	}

	std::ifstream corePak("core.pak", std::ios::binary);
	if (!corePak.is_open()) {
		error("Failed to open core.pak");
		Yngin::terminateYngin();
		return 1;
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

	std::ostringstream coreBytes(std::ios::binary);
	coreBytes << corePak.rdbuf();
	corePak.close();
	ctx->loadCorePak(coreBytes.str().c_str(), coreBytes.str().size());
	coreBytes.clear();

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

	InputSystem* input = ctx->getInputSystem();

	ctx->ready();
	while (ctx->getStatus() == CONTEXT_STATUS::RUNNING) {
#ifdef _WIN32
		if (input->isKeyJustPressed(KEY::F4)) {
			consoleShown = !consoleShown;
			ShowWindow(GetConsoleWindow(), consoleShown ? SW_SHOW : SW_HIDE);
		}
#endif

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
