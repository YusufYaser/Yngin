#include "Editor/Editor.h"
#include "StartWindow/StartWindow.h"
#include <ImGui/imgui.h>
#include <ImGui/imgui_impl_glfw.h>
#include <ImGui/imgui_impl_opengl3.h>
#include <Yngin/Core/Context.h>
#include "main.h"
#include <filesystem>

#ifdef _WIN32
#include <windows.h>
#endif

namespace fs = std::filesystem;

namespace {
	std::vector<Editor*> editors;
	StartWindow* startWindow;
	bool createStartWindow = true;
	std::vector<std::string> openProjectQueue;
}

void showStartWindow() {
	createStartWindow = true;
}

void openProject(std::string path) {
	openProjectQueue.push_back(path);
}

int main(int argc, char* argv[]) {
#if defined(_DEBUG) && defined(_WIN32)
	if (AllocConsole()) {
		SetConsoleTitle(L"Yngin Editor");

		FILE* fDummy;
		freopen_s(&fDummy, "CONOUT$", "w", stdout);
		freopen_s(&fDummy, "CONOUT$", "w", stderr);
		freopen_s(&fDummy, "CONIN$", "r", stdin);
	}
#endif

	IMGUI_CHECKVERSION();

	Yngin::initializeYngin();

	if (!Yngin::isYnginInitialized()) {
		printf("Failed to initialize Yngin\n");
		return 1;
	}

	bool running = true;

	showStartWindow();
	while (running) {
		running = false;

		if (createStartWindow) {
			createStartWindow = false;
			if (startWindow == nullptr) {
				startWindow = new StartWindow();
			} else {
				glfwFocusWindow(startWindow->window);
			}
		}

		for (auto& path : openProjectQueue) {
			Editor* editor = new Editor(path);
			if (editor->ctx->getStatus() == Yngin::CONTEXT_STATUS::RUNNING) {
				editors.push_back(editor);
			} else {
				delete editor;
				editor = nullptr;
			}
		}
		openProjectQueue.clear();

		for (auto& editor : editors) {
			if (editor && editor->ctx->getStatus() == Yngin::CONTEXT_STATUS::RUNNING) {
				running = true;
				fs::path oldCwd = fs::current_path();
				editor->update();
				fs::current_path(oldCwd);
			} else {
				delete editor;
				editor = nullptr;
			}
		}

		if (startWindow != nullptr && !startWindow->shouldClose()) {
			running = true;
			startWindow->update();
		} else {
			delete startWindow;
			startWindow = nullptr;
		}
	}

	delete startWindow;
	startWindow = nullptr;

	for (auto& editor : editors) {
		delete editor;
		editor = nullptr;
	}
	editors.clear();

	Yngin::terminateYngin();

	return 0;
}

#ifdef _WIN32
int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR pCmdLine, int nCmdShow) {
	return main(0, 0);
}
#endif
