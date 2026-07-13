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
		return 1;
	}

	std::ifstream gamePak("game.pak", std::ios::binary);
	if (!gamePak.is_open()) {
		error("Failed to open game.pak");
		Yngin::terminateYngin();
		return 1;
	}

	std::ostringstream gameBytes(std::ios::binary);
	gameBytes << gamePak.rdbuf();
	gamePak.close();

	if (!Context::validateGamePak(gameBytes.str().c_str(), gameBytes.str().size())) {
		error("The game.pak file is corrupted or there's a version mismatch between the game.pak and the runtime executable");
		gameBytes.clear();
		terminateYngin();
		return 1;
	}

	std::unique_ptr<Context> ctx(Context::createContext());

	if (ctx == nullptr || ctx->getStatus() != CONTEXT_STATUS::WAITING_FOR_READY) {
		error("Failed to create a Yngin context");
		gameBytes.clear();

		ctx.reset();

		Yngin::terminateYngin();
		return 1;
	}

	ctx->loadGamePak(gameBytes.str().c_str(), gameBytes.str().size());
	gameBytes.clear();

	Window* window = ctx->getWindow();
	window->setPositionCentered();

	InputSystem* input = ctx->getInputSystem();

	ctx->ready();
	while (ctx->getStatus() == CONTEXT_STATUS::RUNNING) {
#ifdef _WIN32
		if (input->isKeyJustPressed(KEY::F4)) {
			consoleShown = !consoleShown;
			bool wasFocused = window->isFocused();
			ShowWindow(GetConsoleWindow(), consoleShown ? SW_SHOW : SW_HIDE);
			if (wasFocused) window->setFocused();
		}
#endif

		ctx->update();
	}

	ctx.reset();

	Yngin::terminateYngin();

	return 0;
}

#ifdef _WIN32
int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR pCmdLine, int nCmdShow) {
	return main();
}
#endif
