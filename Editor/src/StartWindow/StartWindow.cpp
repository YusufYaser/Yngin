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
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>
#include <dwmapi.h>
#pragma comment(lib, "dwmapi.lib")
#undef min
#undef max
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

	int windowWidth, windowHeight;

	glfwGetWindowSize(window, &windowWidth, &windowHeight);
	glfwSetWindowPos(window, (mode->width - windowWidth) / 2, (mode->height - windowHeight) / 2);

#ifdef _WIN32
	BOOL darkMode = TRUE;
	DwmSetWindowAttribute(glfwGetWin32Window(window), 20, &darkMode, sizeof(BOOL));
#endif

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

	ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0);
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
			75
		};

		ImGui::SetCursorPos(pos);
		ImGui::Text(text.c_str());

		ImGui::PopFont();
	}

	// Recent Projects
	{
		{
			std::string text = "Recent Projects";

			ImVec2 size = ImGui::CalcTextSize(text.c_str());
			ImVec2 pos = ImVec2{
				windowSize.x / 2 - size.x / 2,
				150
			};

			ImGui::SetCursorPos(pos);
			ImGui::Text(text.c_str());
		}

		ImGui::SetNextWindowSize(ImVec2(400, 200));
		ImGui::Begin("Recent Projects", 0, windowFlags);

		for (int i = 0; i < 10; i++) {
			ImGui::BeginGroup();

			ImVec2 p = ImGui::GetCursorScreenPos();
			ImGui::Selectable(("##RecentProject" + std::to_string(i)).c_str(), false, 0, ImVec2(400, 50));
			ImGui::SetCursorScreenPos(p);

			ImGui::Text(("Project #" + std::to_string(i + 1)).c_str());
			ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.5f, 1), "D:\\Codes\\Yngin\\Editor\\TestProject.user");
			ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.5f, 1), "Last Opened: 08/05/2026 9:12 PM");

			ImGui::EndGroup();

			if (ImGui::IsItemHovered()) {
				ImGui::SetMouseCursor(ImGuiMouseCursor_Hand);
			}
		}


		ImVec2 size = ImGui::GetWindowSize();
		ImVec2 pos = ImVec2{
			(windowSize.x - size.x) / 2,
			(windowSize.y - size.y) / 2
		};

		ImGui::SetWindowPos(pos);
		ImGui::SetWindowSize(size);

		ImGui::End();
	}

	// Other Options
	{

		ImGui::PushFont(defaultFont);

		ImGui::Begin("Other Options", 0, windowFlags | ImGuiWindowFlags_AlwaysAutoResize);

		if (ImGui::Button("Open Another Project", ImVec2(250, 25))) {
			std::string path = openDirectory(window);
			if (!path.empty()) {
				if (fs::exists(path) && fs::is_directory(path)) {
					printf("[Yngin Editor] Opening project: %s\n", path.c_str());
					openProject(path);
					closing = true;
				}
			}
		}

		ImGui::Dummy(ImVec2(0, 10));

		if (ImGui::Button("Create a new project", ImVec2(250, 50))) {
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

		ImVec2 size = ImGui::GetWindowSize();
		ImVec2 pos = ImVec2{
			(windowSize.x - size.x) / 2,
			windowSize.y - size.y - 50
		};

		ImGui::SetWindowPos(pos);

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

	ImGui::PopStyleVar();

	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
	glfwSwapBuffers(window);
}

bool StartWindow::shouldClose() const {
	return closing || glfwWindowShouldClose(window) == GLFW_TRUE;
}
