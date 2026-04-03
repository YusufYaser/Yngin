#include "Editor/Editor.h"
#include <ImGui/imgui.h>
#include <Yngin/Core/Context.h>

#ifdef _WIN32
#include <windows.h>
#endif

int main() {
	IMGUI_CHECKVERSION();

#ifdef _DEBUG
	if (AllocConsole()) {
		FILE* fDummy;
		freopen_s(&fDummy, "CONOUT$", "w", stdout);
		freopen_s(&fDummy, "CONOUT$", "w", stderr);
		freopen_s(&fDummy, "CONIN$", "r", stdin);
	}
#endif

	Yngin::initializeYngin();

	if (!Yngin::isYnginInitialized()) {
		printf("Failed to initialize Yngin\n");
		return 1;
	}

	Editor* editor = new Editor();
	while (editor->ctx->getStatus() == Yngin::CONTEXT_STATUS::RUNNING) editor->update();

	delete editor;
	editor = nullptr;

	Yngin::terminateYngin();

	return 0;
}

#ifdef _WIN32
int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR pCmdLine, int nCmdShow) {
	return main();
}
#endif
