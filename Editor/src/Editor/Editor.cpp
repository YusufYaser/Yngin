#include <stdio.h>
#include <Yngin/Yngin.h>
#include <ImGui/imgui.h>
#include <ImGui/imgui_impl_opengl3.h>
#include <ImGui/imgui_impl_glfw.h>
#include <ImGui/imgui_internal.h>
#include <GLFW/glfw3.h>
#include <fstream>
#include <sstream>
#include <format>
#include <filesystem>
#include "Editor.h"
#include "../main.h"
#include "Windows/Properties/PropertiesWindow.h"
#include "Windows/ScriptOutput/ScriptOutputWindow.h"
#include "Windows/Performance/PerformanceWindow.h"
#include "Windows/ContextInfo/ContextInfoWindow.h"
#include "Windows/SceneExplorer/SceneExplorerWindow.h"

#ifdef _WIN32
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>
#undef APIENTRY
#define NOMINMAX
#include <Windows.h>
#include <psapi.h>
#include <dwmapi.h>
#pragma comment(lib, "dwmapi.lib")
#undef min
#undef max
#endif

using namespace Yngin;

namespace fs = std::filesystem;

Editor::Editor(std::string path) {
	this->path = path;

#ifdef _WIN32
	{
		std::string pathForMux = fs::canonical(path).string();

		for (char& c : pathForMux) {
			if (c == '\\' || c == '/' || c == ':') c = '_';
			c = std::tolower(c);
		}

		std::string name = "Global\\YnginEditor_" + pathForMux;

		mutex = CreateMutexA(NULL, TRUE, name.c_str());
		if (mutex == nullptr) {
			printf("CreateMutex failed: %lu\n", GetLastError());
		}

		if (GetLastError() == ERROR_ALREADY_EXISTS) {
			printf("[Yngin Editor] This project is already opened\n");
			CloseHandle(mutex);
			mutex = 0;
			showStartWindow();
			return;
		}
	}
#endif

	fs::path oldCwd = fs::current_path();
	fs::current_path(path);

	fs::create_directory("temp");
	fs::create_directory("bin");
	fs::create_directory("data");
	fs::create_directory("data/scenes");

	fs::path cwd = fs::current_path();
	projectName = cwd.filename().string();

	GLFWmonitor* monitor = glfwGetPrimaryMonitor();
	const GLFWvidmode* mode = glfwGetVideoMode(monitor);

	ctx = Context::createContext({
		.windowSettings = {
			.size = glm::ivec2(1280, 720),
			.title = "Yngin Editor",
		}
		});

	if (ctx == nullptr || ctx->getStatus() != CONTEXT_STATUS::WAITING_FOR_READY) {
		printf("Failed to create context\n");
		fs::current_path(oldCwd);
		return;
	}

	ctx->getWindow()->setPositionCentered();

	glfwSetWindowSizeLimits(ctx->getWindow()->getGLFWwindow(), 854, 480, GLFW_DONT_CARE, GLFW_DONT_CARE);

#ifdef _WIN32
	BOOL darkMode = TRUE;
	DwmSetWindowAttribute(glfwGetWin32Window(ctx->getWindow()->getGLFWwindow()), 20, &darkMode, sizeof(BOOL));
#endif

	{
		std::ifstream file("data/core.pak", std::ios::binary);
		if (file.is_open()) {
			std::ostringstream bytes(std::ios::binary);
			bytes << file.rdbuf();
			file.close();
			PakLoadSettings settings{};
			settings.applyContextSettings = false;
			settings.loadScripts = false;

			ctx->pushLoadPakSettings(settings);
			ctx->loadCorePak(bytes.str().c_str(), bytes.str().size());
			ctx->popLoadPakSettings();
			bytes.clear();
		}
	}

	{
		std::ifstream file("data/resources.pak", std::ios::binary);
		if (file.is_open()) {
			std::ostringstream bytes(std::ios::binary);
			bytes << file.rdbuf();
			file.close();
			ctx->loadResourcesPak(bytes.str().c_str(), bytes.str().size());
			bytes.clear();
		}
	}

	ctx->meta.setMeta("#IsEditor", 1);
	ctx->setMaxFPS(0);

	activeScene = ctx->getScenesManager()->createScene();

	{
		std::ifstream file("data/scenes/scene0.pak", std::ios::binary);
		if (file.is_open()) {
			std::ostringstream bytes(std::ios::binary);
			bytes << file.rdbuf();
			file.close();
			PakLoadSettings settings{};
			settings.loadScripts = false;

			ctx->pushLoadPakSettings(settings);
			activeScene = ctx->getScenesManager()->createScene(bytes.str().c_str(), bytes.str().size(), 0, true);
			ctx->popLoadPakSettings();
			bytes.clear();
		}
	}
	activeScene->activate();

	editorCamera = activeScene->getCamerasManager()->getCamera(0);

	ctx->getPhysicsEngine()->setSimulationEnabled(false);
	ctx->getRenderer()->setLightingEnabled(false);

	setupViewerScene();

	{
		std::ifstream file("data/scripts_editor.pak", std::ios::binary);
		if (file.is_open()) {
			ScriptFileHeader header{};
			file.read(reinterpret_cast<char*>(&header), sizeof(ScriptFileHeader));

			if (std::memcmp(header.magic, "YNGINEDITORSCRIPTS", 10) == 0 && header.version == 0) {
				for (uint32_t i = 0; i < header.scriptsCount; i++) {
					if (!file.good()) break;
					ScriptInfo info{};
					file.read(reinterpret_cast<char*>(&info), sizeof(ScriptInfo));

					char* bytes = new char[info.nameSize];
					file.read(bytes, info.nameSize);

					std::string name(bytes, info.nameSize);

					delete[] bytes;

					bytes = new char[info.scriptSize];
					file.read(bytes, info.scriptSize);

					std::string code(bytes, info.scriptSize);

					delete[] bytes;

					scripts[info.id] = EditorScript{
						.name = name,
						.scene = info.scene,
						.code = code
					};

					nextScriptId = std::max(nextScriptId, info.id + 1);
				}
			}

			file.close();
		}
	}

	lastSaved = ctx->getTime();

	ctx->ready();
	filesLoaded = true;

	ui = std::make_unique<EditorUI>(this);

	// initialize ImGui
	GLFWwindow* glfwWindow = ctx->getWindow()->getGLFWwindow();

	imguiCtx = ImGui::CreateContext();
	ImGui::SetCurrentContext(imguiCtx);
	ImGui_ImplGlfw_InitForOpenGL(glfwWindow, true);
	ImGui_ImplOpenGL3_Init("#version 460");

	ImGuiStyle& style = ImGui::GetStyle();
	style.FrameRounding = 5.0f;
	style.FramePadding = { 3.0f, 3.0f };
	style.DockingNodeHasCloseButton = false;

	ImGuiIO& io = ImGui::GetIO();
	io.IniFilename = nullptr;
	io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
#ifdef _WIN32
	io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;
#endif

	implotCtx = ImPlot::CreateContext();

	saveProject();

	fs::current_path(oldCwd);
}

Editor::~Editor() {
	if (filesLoaded) {
		fs::path oldCwd = fs::current_path();
		fs::current_path(path);

		saveProject();

		fs::current_path(oldCwd);
	}

	if (mutex != 0) CloseHandle(mutex);

	if (implotCtx) {
		ImPlot::SetCurrentContext(implotCtx);
		ImPlot::DestroyContext();
	}

	if (imguiCtx) {
		ImGui::SetCurrentContext(imguiCtx);
		ImGui_ImplOpenGL3_Shutdown();
		ImGui_ImplGlfw_Shutdown();
		ImGui::DestroyContext();
	}

	delete ctx;
	ctx = nullptr;
}

void Editor::resetContext() {
	for (auto& c : ctx->getModelsManager()->getModels()) {
		ctx->getModelsManager()->deleteModel(c);
	}

	for (auto& c : ctx->getMaterialsManager()->getMaterials()) {
		ctx->getMaterialsManager()->deleteMaterial(c);
	}

	for (auto& c : ctx->getScenesManager()->getScenes()) {
		ctx->getScenesManager()->deleteScene(c);
	}

	for (auto& c : ctx->getTexturesManager()->getTextures()) {
		if (c->getId() == 0 || c->getId() == 1 || c->getId() == 2) continue;
		ctx->getTexturesManager()->deleteTexture(c->getId());
	}

	for (auto& c : ctx->getGlobalUIManager()->getElements()) {
		if (c->getId() == 0) continue;
		ctx->getGlobalUIManager()->deleteElement(c);
	}

	for (auto& c : ctx->getScriptsManager()->getScripts()) {
		ctx->getScriptsManager()->deleteScript(c);
	}

	ctx->meta.reset();
	ctx->meta.setMeta("#IsEditor", 1);
	ctx->setMaxFPS(0);
}

void Editor::setupViewerScene() {
	viewerScene = ctx->getScenesManager()->createScene();
	viewerScene->meta.setMeta("#NoExport", 1);

	GameObject* root = viewerScene->getGameObjectsManager()->getRootGameObject();

	viewerObject = root->createChild();
	viewerObject->createComponent<Components::Mesh>();

	viewerLightObject = root->createChild();
	viewerLightObject->setPosition(glm::vec3(0, 0, 5.0f));
	Components::PointLight* light = viewerLightObject->createComponent<Components::PointLight>();
	light->setIntensity(5.0f);
	light->setDistance(24.0f);

	viewerScene->getCamerasManager()->getCamera(0)->setPosition(glm::vec3(1.0f));
	viewerScene->getCamerasManager()->getCamera(0)->lookAt(glm::vec3());

	viewerImage = viewerScene->getUIManager()->getRootElement()->createChild<UI::Image>();
}

void Editor::saveProject() {
	if (running) {
		printf("[Yngin Editor] Cannot save while the game is running!\n");
		return;
	}

	if (saveContext(ctx, scripts)) {
		lastSaved = ctx->getTime();
		printf("[Yngin Editor] Saved Project\n");
	}
}

bool Editor::saveContext(Yngin::Context* ctx, std::map<uint32_t, EditorScript> scripts) {
	ctx->makeCurrent();

	fs::create_directory("temp");
	fs::create_directory("bin");
	fs::create_directory("data/scenes");

	std::ofstream core("data/core.pak", std::ios::binary);
	std::ofstream resources("data/resources.pak", std::ios::binary);
	std::ofstream scriptsFile("data/scripts_editor.pak", std::ios::binary);

	std::map<uint32_t, std::ofstream> scenesFiles;

	bool allScenesFilesOpened = true;

	for (auto& scene : ctx->getScenesManager()->getScenes()) {
		if (scene->meta.getMetaInt("#NoExport", 0) == 1) continue;

		scenesFiles[scene->getId()] = std::ofstream(("data/scenes/scene" + std::to_string(scene->getId()) + ".pak").c_str(), std::ios::binary);

		if (!scenesFiles[scene->getId()].is_open()) allScenesFilesOpened = false;
	}

	if (!allScenesFilesOpened || !core.is_open() || !resources.is_open() || !scriptsFile.is_open()) {
		core.close();
		resources.close();
		scriptsFile.close();

		for (auto& [id, file] : scenesFiles) {
			file.close();
		}
		scenesFiles.clear();

		return false;
	}

	{
		std::vector<char> bytes = ctx->generateCorePak();
		core.write(reinterpret_cast<const char*>(bytes.data()), bytes.size());
		core.close();
	}

	{
		std::vector<char> bytes = ctx->generateResourcesPak();
		resources.write(reinterpret_cast<const char*>(bytes.data()), bytes.size());
		resources.close();
	}

	{
		ScriptFileHeader header{};
		strcpy_s(header.magic, 19, "YNGINEDITORSCRIPTS");
		header.version = 0;
		header.scriptsCount = (uint32_t)scripts.size();
		scriptsFile.write(reinterpret_cast<const char*>(&header), sizeof(ScriptFileHeader));

		for (auto& [id, script] : scripts) {
			ScriptInfo info{};
			info.id = id;
			info.nameSize = script.name.length();
			strcpy_s(info.name, sizeof(info.name), script.name.c_str());
			info.scene = script.scene;
			info.scriptSize = script.code.length();
			scriptsFile.write(reinterpret_cast<const char*>(&info), sizeof(ScriptInfo));
			scriptsFile << script.name;
			scriptsFile << script.code;
		}
		scriptsFile.close();
	}

	{
		for (auto& scene : ctx->getScenesManager()->getScenes()) {
			if (scene->meta.getMetaInt("#NoExport", 0) == 1) continue;

			std::ofstream& file = scenesFiles[scene->getId()];
			if (file.is_open()) {
				std::vector<char> bytes = scene->generatePak();
				file.write(reinterpret_cast<const char*>(bytes.data()), bytes.size());
				file.close();
			}
		}
	}

	return true;
}

void Editor::exportGame() {
	if (running) {
		printf("[Yngin Editor] Cannot export while the game is running!\n");
		return;
	}
	setupPreviousGameState();

	loadScripts();

	ctx->getScriptsManager()->createScript(std::format(R"LUA(
if Yngin.Context.meta:getMetaInt("#IsEditor", 0) == 1 then
	return
end

function onReady()
	Yngin.ScenesManager:setActive({})
	
	Yngin.ScriptsManager:deleteScript(Script.ID)
end
)LUA", 0).c_str());

	{
		std::ofstream file("bin/game.pak", std::ios::binary);
		if (file) {
			PakGenSettings settings{};

			settings.ignoredMetaPrefixes = { "Editor." };
			settings.forceContextSettings = true;

			ContextSettings ctxSettings{};
			memcpy_s(ctxSettings.windowSettings.title, 32, gameSettings.name.c_str(), 32);
			ctxSettings.windowSettings.title[31] = '\0';
			ctxSettings.windowSettings.size = { gameSettings.windowWidth, gameSettings.windowHeight };
			ctxSettings.windowSettings.fullScreen = gameSettings.fullscreen;
			settings.forcedContextSettings = ctxSettings;

			ctx->pushGenPakSettings(settings);
			std::vector<char> bytes = ctx->generateGamePak();
			ctx->popGenPakSettings();

			file.write(reinterpret_cast<const char*>(bytes.data()), bytes.size());
			file.close();
		}
	}


	loadPreviousGameState();
}

void Editor::togglePlayMode() {
	running = !running;

	if (running) {
		explorerSelectionBeforePlaying = explorerSelection;
		explorerSelection = {};

		setupPreviousGameState();
		activeScene->activate();
		ctx->getScenesManager()->deleteScene(viewerScene);
		ctx->meta.setMeta("#IsPlaying", 1);
		ctx->notReady();
		loadScripts();

		runningStartTime = ctx->getFrameStartTime();

		ctx->ready();
	} else {
		if (explorerSelection.first == EXPLORER_SELECTION_TYPE::NONE) {
			explorerSelection = explorerSelectionBeforePlaying;
		}

		loadPreviousGameState();
	}
}

void Editor::setupPreviousGameState() {
	std::ofstream file("temp/previous_game_state.pak", std::ios::binary);
	if (file) {
		std::vector<char> bytes = ctx->generateGamePak();
		file.write(reinterpret_cast<const char*>(bytes.data()), bytes.size());
		file.close();
	}
}

void Editor::loadPreviousGameState() {
	std::ifstream gamePak("temp/previous_game_state.pak", std::ios::binary);
	if (gamePak.is_open()) {
		uint32_t activeSceneId = activeScene->getId();
		uint32_t editorCameraId = editorCamera->getId();

		resetContext();

		std::ostringstream gameBytes(std::ios::binary);
		gameBytes << gamePak.rdbuf();
		gamePak.close();

		std::remove("temp/previous_game_state.pak");

		Window* window = ctx->getWindow();
		std::string title = window->getTitle();
		glm::ivec2 pos = window->getPosition();
		glm::ivec2 size = window->getSize();
		bool fullscreen = window->isFullscreen();

		PakLoadSettings settings{};
		settings.applyContextSettings = false;

		ctx->pushLoadPakSettings(settings);
		ctx->loadGamePak(gameBytes.str().c_str(), gameBytes.str().size());
		ctx->popLoadPakSettings();

		gameBytes.clear();

		window->setTitle(title.c_str());
		window->setPosition(pos);
		window->setSize(size);
		window->setFullscreen(fullscreen);

		activeScene = ctx->getScenesManager()->getScene(activeSceneId);
		editorCamera = activeScene->getCamerasManager()->getCamera(editorCameraId);

		activeScene->activate();

		setupViewerScene();
	}
}

void Editor::loadScripts() {
	for (auto& [id, script] : scripts) {
		Scene* scene = ctx->getScenesManager()->getScene(script.scene);
		ctx->getScriptsManager()->createScript(scene, script.code.c_str(), id, true);
	}
}

void Editor::update() {
	fs::path oldCwd = fs::current_path();
	fs::current_path(path);

	ImGui::SetCurrentContext(imguiCtx);
	ImPlot::SetCurrentContext(implotCtx);
	ctx->makeCurrent();

	InputSystem* input = ctx->getInputSystem();

	ImGuiIO& io = ImGui::GetIO();
	ImGuiStyle& style = ImGui::GetStyle();

	input->setMouseInputsEnabled(!io.WantCaptureMouse && !ImGui::IsPopupOpen("", ImGuiPopupFlags_AnyPopupId | ImGuiPopupFlags_AnyPopupLevel));
	input->setKeyboardInputsEnabled(!io.WantCaptureKeyboard && !ImGui::IsPopupOpen("", ImGuiPopupFlags_AnyPopupId | ImGuiPopupFlags_AnyPopupLevel));

	Window* window = ctx->getWindow();
	window->setTitle((projectName + " - Yngin Editor").c_str());

	glm::ivec2 windowSize = window->getSize();
	if (!ImGui::GetDragDropPayload()) {
		if (!viewingObject) {
			handleCameraMovement(editorCamera);
		} else {
			handleCameraMovement(viewerScene->getCamerasManager()->getCamera(0));
		}
	}

	if (input->isKeyJustPressed(KEY::F1)) {
		system("start https://github.com/YusufYaser/Yngin/wiki");
	}

	if (input->isKeyJustPressed(KEY::F5)) {
		togglePlayMode();
	}

	//if (input->isKeyJustPressed(Yngin::KEY::F11) || (input->isKeyPressed(Yngin::KEY::RALT) && input->isKeyJustPressed(Yngin::KEY::ENTER))) {
		//window->setFullscreen(!window->isFullscreen());
	//}

	Components::Mesh* targetMesh = nullptr;
	bool viewingTextureOnMesh = false;
	uint32_t highlightedMeshOldTexId = 0;

	{
		uint32_t id = ctx->getRenderer()->getGameObjectId(input->getMousePosition());
		if (id != 0) {
			GameObject* obj = activeScene->getGameObjectsManager()->getGameObject(id);
			if (obj) targetMesh = obj->getComponent<Components::Mesh>();
		}
	}

	{
		const ImGuiPayload* payload = ImGui::GetDragDropPayload();

		static bool wasDragging = false;
		static uint32_t texId = -1;

		if (payload && payload->IsDataType("TEXTURE_ID")) {
			wasDragging = true;

			if (targetMesh) {
				texId = *(const uint32_t*)payload->Data;

				viewingTextureOnMesh = true;
				highlightedMeshOldTexId = targetMesh->getTexture();
				targetMesh->setTexture(texId);
			}
		} else if (wasDragging) {
			wasDragging = false;

			if (targetMesh) {
				targetMesh->setTexture(texId);
			}
		}
	}

	{
		float time = ctx->getFrameStartTime();
		if (time - lastGraphTime > 0.1f) {
			lastGraphTime = time;

			if (!graphsTimes.empty() && time - graphsTimes[0] > 10) {
				while (!graphsTimes.empty() && time - graphsTimes[0] > 7) {
					graphsTimes.erase(graphsTimes.begin());
					graphsFPSValues.erase(graphsFPSValues.begin());
					graphsMemoryValues.erase(graphsMemoryValues.begin());
				}
			}

			graphsTimes.push_back(time);
			graphsFPSValues.push_back(1.0f / ctx->getDeltaTime());

#ifdef _WIN32
			PROCESS_MEMORY_COUNTERS pmc;
			if (GetProcessMemoryInfo(GetCurrentProcess(), &pmc, sizeof(pmc))) {
				graphsMemoryValues.push_back(pmc.WorkingSetSize / (1024.0f * 1024.0f)); // Memory in MB
			} else {
				graphsMemoryValues.push_back(0);
			}
#else
			graphsMemoryValues.push_back(0);
#endif
		}
	}

	ctx->getPhysicsEngine()->setSimulationEnabled(running);
	ctx->getRenderer()->setLightingEnabled(running || (explorerSelection.first == EXPLORER_SELECTION_TYPE::MATERIAL && explorerSelection.second != -1) || editorLighting);
	ctx->update(false);

	if (viewingTextureOnMesh && targetMesh) {
		targetMesh->setTexture(highlightedMeshOldTexId);
	}

	if (!running && !viewingObject && !input->isMousePressed(Yngin::MOUSE_BUTTON::RIGHT) && input->isMouseJustPressed(Yngin::MOUSE_BUTTON::LEFT)) {
		if (targetMesh) {
			explorerSelection = { EXPLORER_SELECTION_TYPE::GAMEOBJECT, targetMesh->getGameObject()->getId() };
		} else {
			explorerSelection = { EXPLORER_SELECTION_TYPE::NONE, 0 };
		}
	}

	io.DisplaySize = ImVec2((float)windowSize.x, (float)windowSize.y);

	if (!input->isMousePressed(Yngin::MOUSE_BUTTON::RIGHT) && input->isKeyPressed(Yngin::KEY::LCTRL) && input->isKeyJustPressed(Yngin::KEY::S)) {
		saveProject();
	}

	if (!running && ctx->getTime() - lastSaved > 60) {
		saveProject();
	}

	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();

	float menubarHeight = 0;

	if (ImGui::BeginMainMenuBar()) {
		{
			static ImVec2 prevPos = { -1, -1 };

			if (ImGui::IsItemActive()) {
				if (prevPos.x == -1) {
					prevPos = io.MousePos;
				}

				glm::ivec2 delta = {
					io.MousePos.x - prevPos.x,
					io.MousePos.y - prevPos.y
				};

				if (delta.x != 0 || delta.y != 0) window->restore();

				window->setPosition(window->getPosition() + delta);

				prevPos = io.MousePos;
			} else {
				prevPos = { -1, -1 };
			}
		}

		if (window->isFullscreen() || !window->hasTitleBar()) {
			ImGui::Text(window->getTitle());
			ImGui::Separator();
		}

		if (ImGui::BeginMenu("File")) {
			ImGui::MenuItem(projectName.c_str(), 0, false, false);
			ImGui::Separator();
			if (ImGui::MenuItem("Start Window")) {
				showStartWindow();
			}
			ImGui::Separator();
			if (ImGui::MenuItem("Save", "Ctrl+S")) {
				saveProject();
			}
			ImGui::Separator();
			if (ImGui::MenuItem("Close Project")) {
				ctx->close();
				showStartWindow();
			}
			ImGui::Separator();
			if (ImGui::MenuItem("Close Yngin Editor", "Alt+F4")) {
				ctx->close();
			}
			ImGui::EndMenu();
		}

		if (ImGui::BeginMenu("Edit")) {
			bool canUndo = false;
			bool canRedo = false;

			TextEditor* editor = nullptr;

			uint32_t scriptId = explorerSelection.second;

			auto it = scripts.find(scriptId);
			if (it != scripts.end()) {
				editor = &it->second.editor;
				canUndo = editor->CanUndo();
				canRedo = editor->CanRedo();
			}

			bool enabled = explorerSelection.first == EXPLORER_SELECTION_TYPE::SCRIPT;
			if (ImGui::MenuItem("Undo", "Ctrl+Z", false, enabled && editor && canUndo)) {
				editor->Undo();
			}
			if (ImGui::MenuItem("Redo", "Ctrl+Y", false, enabled && editor && canRedo)) {
				editor->Redo();
			}
			ImGui::Separator();
			if (ImGui::MenuItem("Cut", "Ctrl+X", false, enabled && editor)) {
				editor->Cut();
			}
			if (ImGui::MenuItem("Copy", "Ctrl+C", false, enabled && editor)) {
				editor->Copy();
			}
			if (ImGui::MenuItem("Paste", "Ctrl+V", false, enabled && editor)) {
				editor->Paste();
			}
			if (ImGui::MenuItem("Delete", "Del", false, enabled && editor)) {
				editor->Delete();
			}
			ImGui::Separator();
			if (ImGui::MenuItem("Select All", "Ctrl+A", false, enabled && editor)) {
				editor->SelectAll();
			}
			ImGui::EndMenu();
		}

		if (ImGui::BeginMenu("Game")) {
			if (ImGui::MenuItem("Export game.pak", 0, false, !running)) {
				exportGame();
			}
			ImGui::Separator();
			if (ImGui::MenuItem(running ? "Stop Play Mode" : "Start Play Mode", "F5")) {
				togglePlayMode();
			}
			ImGui::EndMenu();
		}

		{
			bool openAbout = false;

			if (ImGui::BeginMenu("Help")) {
				if (ImGui::MenuItem("GitHub Wiki", "F1")) {
					system("start https://github.com/YusufYaser/Yngin/wiki");
				}
				ImGui::Separator();
				if (ImGui::MenuItem("About Yngin Editor")) {
					openAbout = true;
				}
				ImGui::EndMenu();
			}

			if (openAbout) {
				ImGui::OpenPopup("About Yngin");
			}

			if (ImGui::BeginPopupModal("About Yngin", 0, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove)) {
				ImGui::Text("Yngin Editor - Copyright (c) 2026 Yusuf Kelany");

				ImGui::SetCursorPosX(ImGui::GetCursorPosX() + (ImGui::GetContentRegionAvail().x - (80 * 2 + style.ItemSpacing.x)) / 2.0f);

				if (ImGui::Button("GitHub", ImVec2(80, 30))) {
					system("start https://github.com/YusufYaser/Yngin");
				}
				ImGui::SameLine();
				if (ImGui::Button("Close", ImVec2(80, 30))) {
					ImGui::CloseCurrentPopup();
				}
				ImGui::EndPopup();
			}
		}

		if (window->isFullscreen() || !window->hasTitleBar()) {
			ImGui::SameLine(window->getSize().x - 65.0f * (window->isFullscreen() ? 1.0f : 1.5f));
			ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));
			ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.2f, 0.3f, 0.4f, 1.0f));
			ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 0.0f);
			ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(7.5f, 2.5f));

			if (ImGui::Button("-")) {
				if (window->isFullscreen()) {
					window->setFullscreen(false);
				} else {
					window->minimize();
				}
			}

			if (!window->isFullscreen()) {
				if (ImGui::Button("O")) {
					if (!window->isMaximized()) {
						window->maximize();
					} else {
						window->restore();
					}
				}
			}

			ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(1, 0, 0, 1));
			if (ImGui::Button("X")) {
				ctx->close();
			}
			ImGui::PopStyleColor();

			ImGui::PopStyleColor(2);
			ImGui::PopStyleVar(2);
		}

		if (running) {
			ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1, 0, 0, 1));
			float textWidth = ImGui::CalcTextSize("Running").x;
			ImGui::SetCursorPosX((windowSize.x - textWidth) * 0.5f);
			ImGui::Text("Running");
			ImGui::PopStyleColor();
		}

		menubarHeight = ImGui::GetFrameHeight();

		ImGui::EndMainMenuBar();
	}

	float frameHeight = ImGui::GetFrameHeight();

	ImGuiViewport* viewport = ImGui::GetMainViewport();
	ImVec2 mainViewportPos = viewport->Pos;

	static const ImGuiWindowFlags dockspaceWindowFlags =
		ImGuiWindowFlags_NoDocking |
		ImGuiWindowFlags_NoTitleBar |
		ImGuiWindowFlags_NoCollapse |
		ImGuiWindowFlags_NoResize |
		ImGuiWindowFlags_NoMove;

	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
	ImGui::PushStyleColor(ImGuiCol_DockingEmptyBg, ImVec4(0.05f, 0.05f, 0.05f, 1.0f));

	ImGuiID dockRight = ImGui::GetID("RightDockSpace");
	if (ImGui::BeginViewportSideBar("##RightDockSpaceWindow", viewport, ImGuiDir_Right, 300.0f, dockspaceWindowFlags)) {
		ImGui::DockSpace(dockRight);
		ImGui::End();
	}

	ImGuiID dockLeft = ImGui::GetID("LeftDockSpace");
	if (ImGui::BeginViewportSideBar("##LeftDockSpaceWindow", viewport, ImGuiDir_Left, 250.0f, dockspaceWindowFlags)) {
		ImGui::DockSpace(dockLeft);
		ImGui::End();
	}

	ImGuiID dockBottom = ImGui::GetID("BottomDockSpace");
	if (ImGui::BeginViewportSideBar("##BottomDockSpaceWindow", viewport, ImGuiDir_Down, 260.0f, dockspaceWindowFlags)) {
		ImGui::DockSpace(dockBottom);
		ImGui::End();
	}

	{
		float scaleX = io.DisplayFramebufferScale.x;
		float scaleY = io.DisplayFramebufferScale.y;

		int physicalX = static_cast<int>((viewport->WorkPos.x - viewport->Pos.x) * scaleX);
		int physicalY = static_cast<int>((viewport->WorkPos.y - viewport->Pos.y + frameHeight * 2) * scaleY);
		int physicalWidth = static_cast<int>(viewport->WorkSize.x * scaleX);
		int physicalHeight = static_cast<int>((viewport->WorkSize.y - frameHeight * 2) * scaleY);

		ctx->forceViewport(
			glm::ivec2(physicalX, physicalY),
			glm::ivec2(physicalWidth, physicalHeight)
		);
	}

	ImGui::PopStyleColor(1);
	ImGui::PopStyleVar(1);

	std::vector<EditorWindow*> windowsToRemove;

	viewingObject = false;

	for (auto& window : windows) {
		if (window->shouldClose()) {
			windowsToRemove.push_back(window.get());
			continue;
		}

		window->draw();
	}

	if (ctx->getFrame() == 1) {
		// Right

		PropertiesWindow* properties = new PropertiesWindow(this);
		ImGui::DockBuilderDockWindow(properties->getWindowImGuiId().c_str(), dockRight);
		windows.push_back(std::unique_ptr<PropertiesWindow>(properties));

		// Left

		SceneExplorerWindow* sceneExplorer = new SceneExplorerWindow(this);
		ImGui::DockBuilderDockWindow(sceneExplorer->getWindowImGuiId().c_str(), dockLeft);
		windows.push_back(std::unique_ptr<SceneExplorerWindow>(sceneExplorer));

		// Bottom

		ScriptOutputWindow* scriptOutput = new ScriptOutputWindow(this);
		ImGui::DockBuilderDockWindow(scriptOutput->getWindowImGuiId().c_str(), dockBottom);
		windows.push_back(std::unique_ptr<ScriptOutputWindow>(scriptOutput));

		PerformanceWindow* performanceWindow = new PerformanceWindow(this);
		ImGui::DockBuilderDockWindow(performanceWindow->getWindowImGuiId().c_str(), dockBottom);
		windows.push_back(std::unique_ptr<PerformanceWindow>(performanceWindow));

		ContextInfoWindow* contextInfoWindow = new ContextInfoWindow(this);
		ImGui::DockBuilderDockWindow(contextInfoWindow->getWindowImGuiId().c_str(), dockBottom);
		windows.push_back(std::unique_ptr<ContextInfoWindow>(contextInfoWindow));

	} else if (ctx->getFrame() == 2) {
		ImGui::SetWindowFocus("Window0");
		ImGui::SetWindowFocus("Window1");
	}

	for (auto& window : windowsToRemove) {
		windows.erase(std::remove_if(windows.begin(), windows.end(), [window](const std::unique_ptr<EditorWindow>& w) {
			return w.get() == window;
			}), windows.end());
	}

	if (explorerSelection.first != EXPLORER_SELECTION_TYPE::SCRIPT || explorerSelection.second == -1) {
		std::string title = "Scene Viewer";
		if (running) title = gameSettings.name;
		if (viewingObject) title = "Resource Viewer";

		ImGui::SetNextWindowViewport(viewport->ID);
		ImGui::SetNextWindowPos(ImVec2(mainViewportPos.x + 250.0f, mainViewportPos.y + menubarHeight));
		ImGui::SetNextWindowSize(ImVec2(windowSize.x - 250.0f - 300.0f, 0));

		ImGui::PushStyleColor(ImGuiCol_TitleBg, ImGui::GetStyle().Colors[ImGuiCol_TitleBgActive]);

		if (ImGui::Begin(title.c_str(), 0, ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking)) {

			if (ImGui::BeginMenuBar()) {
				if (!viewingObject) {
					ImGui::PushStyleColor(ImGuiCol_Button, running ? ImVec4(.5f, 0, 0, 1) : ImVec4(0, .5f, 0, 1));
					if (ImGui::Button(((running ? "Stop" : "Start") + std::string("##TogglePlayMode")).c_str())) {
						togglePlayMode();
					}
					ImGui::PopStyleColor();
					if (!running) {
						ImGui::SameLine();
						ImGui::Checkbox("Lighting", &editorLighting);
					}
				} else {
					if (explorerSelection.first == EXPLORER_SELECTION_TYPE::MODEL) {
						Model* model = ctx->getModelsManager()->getModel(explorerSelection.second);
						if (model) {
							const ModelData& d = model->getModelData();

							ImGui::Text("Vertices: %i\tSubmeshes: %i", d.vertices.size(), model->getSubmeshesCount());
						}
					} else if (explorerSelection.first == EXPLORER_SELECTION_TYPE::TEXTURE) {
						Texture* texture = ctx->getTexturesManager()->getTexture(explorerSelection.second);
						if (texture) {
							glm::ivec2 size = texture->getSize();

							ImGui::Text("Size: %ix%i", size.x, size.y);
						}
					}
				}

				{
					char fpsText[32];

					snprintf(fpsText, sizeof(fpsText), "%i FPS", int(1 / ctx->getDeltaTime() + 0.5));

					float textWidth = ImGui::CalcTextSize(fpsText).x;

					ImGui::SameLine(ImGui::GetWindowWidth() - textWidth - style.WindowPadding.x - 10.0f);

					ImGui::Text("%s", fpsText);
				}

				ImGui::EndMenuBar();
			}

			ImGui::End();
		}

		ImGui::PopStyleColor();
	}

	if (viewingObject && !running) {
		viewerScene->activate();
	} else {
		activeScene->activate();
	}

	if (explorerSelection.first == EXPLORER_SELECTION_TYPE::SCRIPT && explorerSelection.second != -1) {
		ctx->forceViewport({ -1, -1 }, { 1, 1 });

		ImGui::SetNextWindowPos(viewport->WorkPos);
		ImGui::SetNextWindowSize(viewport->WorkSize);
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));

		static int prevScriptId = -1;
		uint32_t scriptId = explorerSelection.second;

		auto it = scripts.find(scriptId);
		if (it != scripts.end()) {
			EditorScript& script = it->second;

			if (script.editor.GetLanguageDefinition().mName == "HLSL") {
				script.editor.SetLanguageDefinition(TextEditor::LanguageDefinition::Lua());
				script.editor.SetShowWhitespaces(false);
			}

			std::string title = script.name;

			ImGui::Begin(title.append("###Script Editor").c_str(), nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse);
			{

				if (scriptId != prevScriptId) {
					prevScriptId = scriptId;
					script.editor.SetText(script.code.c_str());
				}

				script.editor.SetReadOnly(running);
				script.editor.Render("Script Editor Code");
				if (script.editor.IsTextChanged()) {
					std::string codeStr = script.editor.GetText();
					codeStr.erase(codeStr.size() - 1); // remove \n at the end
					script.code = codeStr;
				}
				ImGui::End();
			}
		}

		ImGui::PopStyleVar();
	}

	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

#ifdef _WIN32
	if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
		ImGui::UpdatePlatformWindows();
		ImGui::RenderPlatformWindowsDefault();
		ctx->makeCurrent();
	}
#endif

	ctx->swapBuffers();

	fs::current_path(oldCwd);
}
