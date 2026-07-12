#include "StartWindow.h"
#include <string>
#include "../main.h"
#include <filesystem>
#include <chrono>
#include <ctime>
#include <fstream>
#include "../Editor/Editor.h"

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
	uint64_t getUnixTime() {
		auto now = std::chrono::system_clock::now();
		auto duration = now.time_since_epoch();
		return std::chrono::duration_cast<std::chrono::seconds>(duration).count();
	}

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

	ImGuiStyle& style = ImGui::GetStyle();
	style.FrameRounding = 5.0f;

	defaultFont = io.Fonts->AddFontDefault();
	titleFont = io.Fonts->AddFontFromFileTTF("assets/ArchivoBlack.ttf", 48.0f);


	// Load recent projects

	{
		std::ifstream file("recent_projects.dat");

		if (file.is_open()) {
			std::string data;

			while (std::getline(file, data, ';')) {
				RecentProject proj{};

				proj.path = data;

				if (!std::getline(file, data, ';')) break;
				proj.name = data;

				if (!std::getline(file, data)) break;

				try {
					proj.lastOpened = std::stoll(data);

					recentProjects[proj.path] = proj;
				} catch (std::exception) {
					continue;
				}
			}

			file.close();
		}
	}
}

StartWindow::~StartWindow() {
	{
		std::ofstream file("recent_projects.dat", std::ios::trunc);

		if (file.is_open()) {
			for (auto it = recentProjects.rbegin(); it != recentProjects.rend(); it++) {
				RecentProject proj = it->second;

				if (proj.path.find(';') != std::string::npos || proj.name.find(';') != std::string::npos) continue;

				file << proj.path << ";" << proj.name << ";" << proj.lastOpened << "\n";
			}

			file.close();
		}
	}

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

	ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0, 0, 0, 0));

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

		RecentProject* projToOpen = nullptr;

		std::map<uint64_t, RecentProject*> sorted;
		for (auto& [_, proj] : recentProjects) {
			sorted[proj.lastOpened] = &proj;
		}

		for (auto it = sorted.rbegin(); it != sorted.rend(); it++) {
			RecentProject* proj = it->second;

			std::string lastOpened = "08/05/2026 9:12 PM";

			std::tm date;
			localtime_s(&date, (const time_t*)&proj->lastOpened);
			char lastOpenedBuf[128];
			std::strftime(lastOpenedBuf, sizeof(lastOpenedBuf), "%d %b %Y %I:%M %p", &date);
			lastOpened = lastOpenedBuf;

			ImGui::BeginGroup();

			ImVec2 p = ImGui::GetCursorScreenPos();
			ImGui::PushStyleColor(ImGuiCol_HeaderHovered, ImVec4(0.26f, 0.59f, 0.98f, 0.4f));
			if (ImGui::Selectable(("##RecentProject" + proj->path).c_str(), false, 0, ImVec2(400, 50))) {
				projToOpen = proj;
			}
			ImGui::PopStyleColor();

			ImGui::SetCursorScreenPos(p);

			ImGui::Text(proj->name.c_str());
			ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.5f, 1), proj->path.c_str());
			ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.5f, 1), ("Last Opened: " + lastOpened).c_str());

			ImGui::EndGroup();

			if (ImGui::IsItemHovered()) {
				ImGui::SetMouseCursor(ImGuiMouseCursor_Hand);
			}
		}

		if (projToOpen != nullptr) {
			std::string path = projToOpen->path;

			if (fs::exists(path) && fs::is_directory(path)) {
				projToOpen->lastOpened = getUnixTime();

				printf("[Yngin Editor] Opening project from recents: %s\n", path.c_str());
				openProject(path);
				closing = true;
			} else {
				printf("[Yngin Editor] This project no longer exists\n");
				ImGui::OpenPopup("ProjectNotFound");
			}
		}


		ImVec2 size = ImGui::GetWindowSize();
		ImVec2 pos = ImVec2{
			(windowSize.x - size.x) / 2,
			(windowSize.y - size.y) / 2
		};

		ImGui::SetWindowPos(pos);
		ImGui::SetWindowSize(size);

		// Path Not Found
		{
			if (ImGui::BeginPopupModal("Project Not Found###ProjectNotFound", 0, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove)) {
				ImGui::Text("This project no longer exists");

				ImGui::Dummy(ImVec2(0, 5));
				if (ImGui::Button("Close")) {
					ImGui::CloseCurrentPopup();
				}

				ImGui::EndPopup();
			}
		}

		ImGui::End();

		if (recentProjects.size() == 0) {
			std::string text = "No Recent Projects";

			ImVec2 size = ImGui::CalcTextSize(text.c_str());
			ImVec2 pos = ImVec2{
				windowSize.x / 2 - size.x / 2,
				windowSize.y / 2 - size.y / 2,
			};

			ImGui::SetCursorPos(pos);
			ImGui::Text(text.c_str());
		}
	}

	// Other Options
	{

		ImGui::PushFont(defaultFont);

		ImGui::Begin("Other Options", 0, windowFlags | ImGuiWindowFlags_AlwaysAutoResize);

		if (ImGui::Button("Open Another Project", ImVec2(250, 25))) {
			std::string path = openDirectory(window);
			if (!path.empty()) {
				if (fs::exists(path) && fs::is_directory(path)) {
					RecentProject proj;
					proj.path = path;
					proj.name = fs::path(path).filename().string();
					proj.lastOpened = getUnixTime();

					recentProjects[path] = proj;

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

						if (Editor::generateNewProject(path)) {
							RecentProject proj;
							proj.path = path;
							proj.name = fs::path(path).filename().string();
							proj.lastOpened = getUnixTime();

							recentProjects[path] = proj;

							openProject(path);
							closing = true;
						} else {
							printf("[Yngin Editor] Failed to create a new project\n");
						}
					} else {
						printf("[Yngin Editor] Please create an empty directory for a new project\n");
						ImGui::OpenPopup("NonEmptyPath");
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

		// Non-empty Path
		{
			if (ImGui::BeginPopupModal("Error###NonEmptyPath", 0, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove)) {
				ImGui::Text("Please create an empty folder to create a new project");

				ImGui::Dummy(ImVec2(0, 5));
				if (ImGui::Button("Close")) {
					ImGui::CloseCurrentPopup();
				}

				ImGui::EndPopup();
			}
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

	ImGui::PopStyleColor();

	ImGui::End();

	ImGui::PopStyleVar();

	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
	glfwSwapBuffers(window);
}

bool StartWindow::shouldClose() const {
	return closing || glfwWindowShouldClose(window) == GLFW_TRUE;
}
