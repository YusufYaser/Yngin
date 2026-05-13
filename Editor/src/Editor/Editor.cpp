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

#ifdef _WIN32
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>
#undef APIENTRY
#define NOMINMAX
#include <Windows.h>
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

	ctx = createContext({
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
			activeScene = ctx->getScenesManager()->createScene(bytes.str().c_str(), bytes.str().size(), 0, true);
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
	imguiCtx = ImGui::CreateContext();
	ImGui::SetCurrentContext(imguiCtx);
	ImGui_ImplGlfw_InitForOpenGL(ctx->getWindow()->getGLFWwindow(), true);
	ImGui_ImplOpenGL3_Init("#version 460");

	ImGuiStyle& style = ImGui::GetStyle();
	style.FrameRounding = 5.0f;
	style.FramePadding = { 3.0f, 3.0f };

	ImGuiIO& io = ImGui::GetIO();
	io.IniFilename = nullptr;

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
	Components::Light* light = viewerLightObject->createComponent<Components::Light>();
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
	ctx->makeCurrent();

	ImGuiIO& io = ImGui::GetIO();

	Window* window = ctx->getWindow();
	window->setTitle((projectName + " - Yngin Editor").c_str());

	glm::ivec2 windowSize = window->getSize();
	if (!viewingObject) {
		handleCameraMovement(editorCamera);
	} else {
		handleCameraMovement(viewerScene->getCamerasManager()->getCamera(0));
	}

	InputSystem* input = ctx->getInputSystem();

	if (input->isKeyJustPressed(KEY::F1)) {
		system("start https://github.com/YusufYaser/Yngin/wiki");
	}

	if (input->isKeyJustPressed(KEY::F5)) {
		togglePlayMode();
	}

	//if (input->isKeyJustPressed(Yngin::KEY::F11) || (input->isKeyPressed(Yngin::KEY::RALT) && input->isKeyJustPressed(Yngin::KEY::ENTER))) {
		//window->setFullscreen(!window->isFullscreen());
	//}

	/*std::pair<UI::UIElement*, glm::vec4> oldUIColor = {};
	std::pair<Components::Mesh*, glm::vec3> oldObjectColor = {};
	if (!running) {
		for (auto& element : activeScene->getUIManager()->getElements()) {
			if (element->getType() == UI_TYPE::NONE) continue;

			if (element->isClicked()) {
				explorerSelection = { EXPLORER_SELECTION_TYPE::UIELEMENT, element->getId() };
			}
		}

		if (explorerSelection.first == EXPLORER_SELECTION_TYPE::GAMEOBJECT) {
			uint32_t id = explorerSelection.second;
			if (id != -1) {
				GameObject* obj = activeScene->getGameObjectsManager()->getGameObject(id);
				if (obj) {
					Components::Mesh* mesh = obj->getComponent<Components::Mesh>();
					if (mesh) {
						oldObjectColor = { mesh, mesh->getColor() };
						mesh->setColor(mesh->getColor() * glm::vec3(0.5f, 1, 1));
					}
				}
			}
		} else if (explorerSelection.first == EXPLORER_SELECTION_TYPE::UIELEMENT) {
			uint32_t id = explorerSelection.second;
			if (id != -1) {
				UI::UIElement* element = activeScene->getUIManager()->getElement(id);
				if (element) {
					oldUIColor = { element, element->getColor() };
					element->setColor(element->getColor() * glm::vec4(0.5f, 1, 1, 1));
				}
			}
		}
	}*/
	ctx->update(false);
	/*if (!running) {
		if (oldUIColor.first != nullptr) oldUIColor.first->setColor(oldUIColor.second);
		if (oldObjectColor.first != nullptr) oldObjectColor.first->setColor(oldObjectColor.second);
	}*/
	io.DisplaySize = ImVec2((float)windowSize.x, (float)windowSize.y);

	if (!input->isMousePressed(Yngin::MOUSE_BUTTON::RIGHT) && input->isKeyPressed(Yngin::KEY::LCTRL) && input->isKeyJustPressed(Yngin::KEY::S)) {
		saveProject();
	}

	if (!running && ctx->getTime() - lastSaved > 60) {
		saveProject();
	}

	ImGui_ImplOpenGL3_NewFrame();
	ImGui::NewFrame();

	float menubarHeight = 0;

	if (ImGui::BeginMainMenuBar()) {
		{
			static ImVec2 startPos = { -1, -1 };

			if (ImGui::IsItemActive()) {
				if (startPos.x == -1) {
					startPos = io.MousePos;
				}

				glm::ivec2 delta = {
					io.MousePos.x - startPos.x,
					io.MousePos.y - startPos.y
				};

				if (delta.x != 0 || delta.y != 0) window->restore();

				window->setPosition(window->getPosition() + delta);
			} else {
				startPos = { -1, -1 };
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
			static bool showAbout = false;
			if (ImGui::BeginMenu("Help")) {
				if (ImGui::MenuItem("GitHub Wiki", "F1")) {
					system("start https://github.com/YusufYaser/Yngin/wiki");
				}
				ImGui::Separator();
				if (ImGui::MenuItem("About Yngin Editor")) {
					showAbout = true;
				}
				ImGui::EndMenu();
			}

			if (showAbout) {
				ImGui::SetNextWindowSize(ImVec2(360, 75));
				ImGui::SetNextWindowPos(ImVec2(windowSize.x / 2.0f, windowSize.y / 2.0f - 100), 0, ImVec2(0.5f, 0.5f));
				if (ImGui::Begin("About Yngin", &showAbout, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse)) {
					ImGui::Text("Yngin Editor - Copyright (c) 2026 Yusuf Kelany");
					if (ImGui::Button("GitHub")) {
						system("start https://github.com/YusufYaser/Yngin");
					}
					ImGui::SameLine();
					if (ImGui::Button("Close")) {
						showAbout = false;
					}
				}
				ImGui::End();
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

	if (explorerSelection.first != EXPLORER_SELECTION_TYPE::SCRIPT || explorerSelection.second == -1) {
		std::string title = "Scene Viewer";
		if (running) title = gameSettings.name;
		if (viewingObject) title = "Resource Viewer";

		ImGui::SetNextWindowPos(ImVec2(250.0f, menubarHeight));
		ImGui::SetNextWindowSize(ImVec2(windowSize.x - 250.0f - 300.0f, 0));

		ImGui::PushStyleColor(ImGuiCol_TitleBg, ImGui::GetStyle().Colors[ImGuiCol_TitleBgActive]);

		if (ImGui::Begin(title.c_str(), 0, ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_MenuBar)) {

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

				ImGui::EndMenuBar();
			}

			ImGui::End();
		}

		ImGui::PopStyleColor();
	}
	ctx->getPhysicsEngine()->setSimulationEnabled(running);
	ctx->getRenderer()->setLightingEnabled(running || (explorerSelection.first == EXPLORER_SELECTION_TYPE::MATERIAL && explorerSelection.second != -1) || editorLighting);

	ImGuiViewport* viewport = ImGui::GetMainViewport();

	if (ImGui::BeginViewportSideBar("##Explorer", viewport, ImGuiDir_Left, 250.0f, 0)) {
		ImGui::Text("Project Explorer");
		ImGui::Separator();
		ImGui::BeginTabBar("Explorer Tabs");
		if (ImGui::BeginTabItem("Scene")) {
			showSceneExplorer();
			ImGui::EndTabItem();
		}
		if (ImGui::BeginTabItem("Game")) {
			showGameExplorer();
			ImGui::EndTabItem();
		}
		if (ImGui::BeginTabItem("Resources")) {
			showResourceExplorer();
			ImGui::EndTabItem();
		}
		ImGui::EndTabBar();
		ImGui::End();
	}

	viewingObject = false;
	if (ImGui::BeginViewportSideBar("##Properties", viewport, ImGuiDir_Right, 300.0f, ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoNavFocus)) {
		switch (explorerSelection.first) {
		case EXPLORER_SELECTION_TYPE::GAME:
			showGameProps();
			break;

		case EXPLORER_SELECTION_TYPE::SCRIPT:
			showScriptProps(explorerSelection.second);
			break;

		case EXPLORER_SELECTION_TYPE::GAMEOBJECT:
			showGameObjectProps(explorerSelection.second);
			break;

		case EXPLORER_SELECTION_TYPE::UIELEMENT:
		{
			showUIElementProps(explorerSelection.second, false);
			break;
		}

		case EXPLORER_SELECTION_TYPE::MODEL:
		{
			showModelProps(explorerSelection.second);
			break;
		}

		case EXPLORER_SELECTION_TYPE::MATERIAL:
		{
			showMaterialProps(explorerSelection.second);
			break;
		}

		case EXPLORER_SELECTION_TYPE::TEXTURE:
		{
			showTextureProps(explorerSelection.second);
			break;
		}

		case EXPLORER_SELECTION_TYPE::SCENE:
			showSceneProps(explorerSelection.second);
			break;
		}
		ImGui::End();
	}

	if (viewingObject && !running) {
		viewerScene->activate();
	} else {
		activeScene->activate();
	}

	glm::ivec2 viewportSize = ctx->getViewportSize();

	float frameHeight = ImGui::GetFrameHeight();

	ImGui::PushStyleColor(ImGuiCol_TitleBg, ImGui::GetStyle().Colors[ImGuiCol_TitleBgActive]);
	ImGui::SetNextWindowPos(ImVec2(250, windowSize.y - 260.0f));
	ImGui::SetNextWindowSize(ImVec2(windowSize.x - 250 - 300.0f, 260.0f));
	ImGui::Begin("Bottom", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar);

	ImGui::BeginTabBar("Tabs");
	if (ImGui::BeginTabItem("Output")) {
		std::string logs = "";
		int i = 0;

		static char filter[512] = {};

		for (auto& [id, log] : ctx->getScriptsManager()->getGlobalOutput()) {
			if (i++ < logsStart) {
				continue;
			}
			std::string source = id == -1 ? "ScriptsManager" : std::format("Script #{}", id);

			std::string line = "[" + source + "] " + log + "\n";

			if (line.find(std::string(filter)) != std::string::npos) {
				logs.insert(0, line);
			}
		}

		char* temp = new char[logs.size() + 1];
		memcpy(temp, logs.c_str(), logs.size());
		temp[logs.size()] = '\0';

		ImGui::Text("Filter");
		ImGui::SameLine();
		ImGui::InputText("##Filter", filter, sizeof(filter));

		ImGui::SameLine(ImGui::GetWindowWidth() - 50.0f);

		if (ImGui::Button("Clear")) {
			logsStart = i;
		}

		ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0, 0, 0, 0));
		ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 2.0f);
		ImGui::InputTextMultiline("##Output Text", temp, logs.size() + 1, ImVec2(-1, -frameHeight - 4), ImGuiInputTextFlags_ReadOnly);
		ImGui::PopStyleColor();
		ImGui::PopStyleVar();

		delete[] temp;

		{
			static char v[1024] = {};

			ImGui::PushItemWidth(-1);
			if (ImGui::InputText("##Global Execute", v, IM_ARRAYSIZE(v), ImGuiInputTextFlags_EnterReturnsTrue)) {
				ImGui::SetKeyboardFocusHere(-1);
				ctx->getScriptsManager()->execute(v);
				v[0] = '\0';
			}
			ImGui::PopItemWidth();
		}

		ImGui::EndTabItem();
	}
	if (ImGui::BeginTabItem("Textures")) {
		auto textures = ctx->getTexturesManager()->getTextures();

		int maxElementsPerRow = (windowSize.x - 250 - 300.0f) / 144;

		int i = 0;
		for (auto& tex : textures) {
			if (i % maxElementsPerRow != 0) ImGui::SameLine();

			GLuint GLid = tex->getGLid();

			if (ImGui::ImageButton(std::string("Texture #" + std::to_string(tex->getId())).c_str(), (void*)(intptr_t)GLid, ImVec2(128, 128))) {
				//explorerSelection = { EXPLORER_SELECTION_TYPE::TEXTURE, tex->getId() };
			}
			if (ImGui::BeginDragDropSource()) {
				uint32_t id = tex->getId();
				ImGui::SetDragDropPayload("TEXTURE_ID", &id, sizeof(uint32_t));

				ImGui::Image((void*)(intptr_t)GLid, ImVec2(128, 128));
				ImGui::Text("Texture #%i", tex->getId());

				ImGui::EndDragDropSource();
			}

			i++;
		}
		ImGui::NewLine();

		ImGui::EndTabItem();
	}
	if (ImGui::BeginTabItem("Debug")) {
		if (ImGui::BeginTable("Context Info", 3, ImGuiTableFlags_Borders, ImVec2(ImGui::GetContentRegionAvail().x * 0.5f, 0))) {
			ImGui::TableSetupColumn("Name");
			ImGui::TableSetupColumn("Value");
			ImGui::TableSetupColumn("Max");

			ImGui::TableHeadersRow();

			std::vector<std::tuple<std::string, size_t, size_t>> rows;

			rows.push_back({ "Models", ctx->getModelsManager()->getModelsCount(), ctx->getModelsManager()->getMaxModelsCount() });
			rows.push_back({ "Materials", ctx->getMaterialsManager()->getMaterialsCount(), ctx->getMaterialsManager()->getMaxMaterialsCount() });
			rows.push_back({ "Textures", ctx->getTexturesManager()->getTexturesCount(), ctx->getTexturesManager()->getMaxTexturesCount() });
			rows.push_back({ "Loaded Scripts", ctx->getScriptsManager()->getScriptsCount(), 0 });

			rows.push_back({ "Global UI Elements", ctx->getGlobalUIManager()->getElementsCount(), 0 });

			for (auto& row : rows) {
				ImGui::TableNextRow();

				ImGui::TableSetColumnIndex(0);
				ImGui::Text("%s", std::get<0>(row).c_str());
				ImGui::TableSetColumnIndex(1);
				ImGui::Text("%i", std::get<1>(row));
				ImGui::TableSetColumnIndex(2);
				if (std::get<2>(row) != 0) ImGui::Text("%i", std::get<2>(row));
			}

			ImGui::EndTable();
		}

		ImGui::SameLine();

		if (ImGui::BeginTable("Scene Info", 3, ImGuiTableFlags_Borders, ImVec2(ImGui::GetContentRegionAvail().x, 0))) {
			ImGui::TableSetupColumn("Name");
			ImGui::TableSetupColumn("Value");
			ImGui::TableSetupColumn("Max");

			ImGui::TableHeadersRow();

			std::vector<std::tuple<std::string, size_t, size_t>> rows;

			rows.push_back({ "Game Objects", activeScene->getGameObjectsManager()->getGameObjectsCount(), 0 });
			rows.push_back({ "Scene UI Elements", activeScene->getUIManager()->getElementsCount(), 0 });
			rows.push_back({ "Submeshes Rendered", ctx->getRenderer()->getSubmeshesRendered(), 0 });

			for (auto& row : rows) {
				ImGui::TableNextRow();

				ImGui::TableSetColumnIndex(0);
				ImGui::Text("%s", std::get<0>(row).c_str());
				ImGui::TableSetColumnIndex(1);
				ImGui::Text("%i", std::get<1>(row));
				ImGui::TableSetColumnIndex(2);
				if (std::get<2>(row) != 0) ImGui::Text("%i", std::get<2>(row));
			}

			ImGui::EndTable();
		}

		ImGui::EndTabItem();
	}
	ImGui::EndTabBar();
	ImGui::End();
	ImGui::PopStyleColor();

	glm::vec2 viewPos = { 250.0f, menubarHeight };
	glm::vec2 viewSize = { windowSize.x - 250.0f - 300.0f, windowSize.y - menubarHeight - 260.0f };
	ctx->forceViewport(viewPos + glm::vec2(0, frameHeight * 2), viewSize - glm::vec2(0, frameHeight * 2));
	if (explorerSelection.first == EXPLORER_SELECTION_TYPE::SCRIPT && explorerSelection.second != -1) {
		ctx->forceViewport({ -1, -1 }, { 1, 1 });

		ImGui::SetNextWindowPos({ viewPos.x, viewPos.y });
		ImGui::SetNextWindowSize({ viewSize.x, viewSize.y });
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

	ctx->swapBuffers();

	fs::current_path(oldCwd);
}
