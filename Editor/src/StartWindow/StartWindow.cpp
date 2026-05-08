#include "StartWindow.h"
#include <string>
#include "../main.h"
#include <filesystem>

#define IMGUI_IMPL_OPENGL_LOADER_GL3W
#include <GLFW/glfw3.h>
#include <ImGui/imgui.h>
#include <ImGui/imgui_impl_glfw.h>
#include <ImGui/imgui_impl_opengl3.h>

#ifdef _WIN32
#include <Windows.h>
#include <shobjidl.h> 
#include <GLFW/glfw3.h>
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>
#endif

namespace fs = std::filesystem;

namespace {
	std::string openDirectory(GLFWwindow* window) {
#ifdef _WIN32
		HWND hwnd = glfwGetWin32Window(window);
		char path[256] = { 0 };

		HRESULT hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);
		if (!SUCCEEDED(hr)) return "";

		IFileOpenDialog* fileOpen;

		hr = CoCreateInstance(CLSID_FileOpenDialog, NULL, CLSCTX_ALL, IID_IFileOpenDialog, reinterpret_cast<void**>(&fileOpen));
		if (!SUCCEEDED(hr)) {
			CoUninitialize();
			return "";
		}

		DWORD dwOptions;
		fileOpen->GetOptions(&dwOptions);
		fileOpen->SetOptions(dwOptions | FOS_PICKFOLDERS);

		hr = fileOpen->Show(hwnd);

		if (!SUCCEEDED(hr)) {
			fileOpen->Release();
			CoUninitialize();
			return "";
		}

		IShellItem* item;

		hr = fileOpen->GetResult(&item);
		if (!SUCCEEDED(hr)) {
			item->Release();
			fileOpen->Release();
			CoUninitialize();
			return "";
		}

		PWSTR filePath;
		hr = item->GetDisplayName(SIGDN_FILESYSPATH, &filePath);

		if (SUCCEEDED(hr)) {
			wcstombs_s(0, path, filePath, 256);
			CoTaskMemFree(filePath);
		}

		item->Release();
		fileOpen->Release();
		CoUninitialize();

		return std::string(path);
#else
		return "";
#endif
	}
}

StartWindow::StartWindow() {
	glfwDefaultWindowHints();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

	window = glfwCreateWindow(800, 600, "Yngin Editor", nullptr, nullptr);

	GLFWmonitor* monitor = glfwGetPrimaryMonitor();
	const GLFWvidmode* mode = glfwGetVideoMode(monitor);

	glfwSetWindowPos(window, (mode->width - 800) / 2, (mode->height - 600) / 2);

	glfwDefaultWindowHints();

	glfwMakeContextCurrent(window);
	glfwSwapInterval(1);

	imguiCtx = ImGui::CreateContext();
	ImGui::SetCurrentContext(imguiCtx);

	ImGui_ImplGlfw_InitForOpenGL(window, true);
	ImGui_ImplOpenGL3_Init("#version 460");

	ImGuiIO& io = ImGui::GetIO();
	io.IniFilename = nullptr;

	defaultFont = io.Fonts->AddFontDefault();
	titleFont = io.Fonts->AddFontFromFileTTF("assets/ArchivoBlack.ttf", 48.0f);
}

StartWindow::~StartWindow() {
	ImGui::SetCurrentContext(imguiCtx);
	glfwMakeContextCurrent(window);
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();

	glfwDestroyWindow(window);
}

void StartWindow::update() {
	ImGui::SetCurrentContext(imguiCtx);
	glfwMakeContextCurrent(window);
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();

	glfwPollEvents();

	ImVec2 windowSize = ImGui::GetIO().DisplaySize;

	ImGuiWindowFlags windowFlags = ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar;

	ImGui::GetBackgroundDrawList()->AddRectFilled(
		ImVec2(0, 0),
		windowSize,
		ImGui::GetColorU32(ImGuiCol_WindowBg)
	);

	ImGui::SetNextWindowPos({ 0, 0 });
	ImGui::SetNextWindowSize(windowSize);
	ImGui::Begin("Start Window", 0, windowFlags | ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav | ImGuiWindowFlags_NoInputs);

	// Title
	{
		std::string text = "Yngin Editor";

		ImGui::PushFont(titleFont);

		ImVec2 size = ImGui::CalcTextSize(text.c_str());
		ImVec2 pos = ImVec2{
			windowSize.x / 2 - size.x / 2,
			100
		};

		ImGui::SetCursorPos(pos);
		ImGui::Text(text.c_str());

		ImGui::PopFont();
	}

	// Options
	{
		ImVec2 size = { 250, 175 };
		ImVec2 pos = ImVec2{
			(windowSize.x - size.x) / 2,
			(windowSize.y - size.y) / 2
		};

		ImGui::PushFont(defaultFont);

		ImGui::SetNextWindowSize(size);
		ImGui::SetNextWindowPos(pos);

		ImGui::Begin("Options", 0, windowFlags);

		if (ImGui::Button("Open Project", ImVec2(-1, 50))) {
			std::string path = openDirectory(window);
			if (!path.empty()) {
				if (fs::exists(path) && fs::is_directory(path)) {
					printf("[Yngin Editor] Opening project: %s\n", path.c_str());
					openProject(path);
					closing = true;
				}
			}
		}

		if (ImGui::Button("New Project", ImVec2(-1, 50))) {
			std::string path = openDirectory(window);
			if (!path.empty()) {
				if (fs::exists(path) && fs::is_directory(path)) {
					if (fs::is_empty(path)) {
						printf("[Yngin Editor] Creating new project at: %s\n", path.c_str());
						openProject(path);
						closing = true;
					} else {
						printf("[Yngin Editor] Please create an empty directory for a new project\n");
					}
				}
			}
		}

		if (ImGui::Button("Close", ImVec2(-1, 50))) {
			closing = true;
		}

		ImGui::End();

		ImGui::PopFont();
	}

	// About
	{
		std::string text = "Yngin Editor - Copyright (c) 2026 Yusuf Kelany";

		ImGui::PushFont(defaultFont);

		ImVec2 size = ImGui::CalcTextSize(text.c_str());
		ImVec2 pos = ImVec2{
			windowSize.x / 2 - size.x / 2,
			windowSize.y - size.y - 10
		};

		ImGui::SetCursorPos(pos);
		ImGui::Text(text.c_str());

		ImGui::PopFont();
	}

	ImGui::End();

	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
	glfwSwapBuffers(window);
}

bool StartWindow::shouldClose() const {
	return closing || glfwWindowShouldClose(window) == GLFW_TRUE;
}
