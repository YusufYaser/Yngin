#include <stdio.h>
#include <Yngin/Yngin.h>
#include <ImGui/imgui.h>
#include <ImGui/imgui_impl_opengl3.h>
#include <ImGui/imgui_impl_glfw.h>
#include <ImGui/imgui_internal.h>
#include <GLFW/glfw3.h>
#include "Cube_Model.h"
#include <fstream>
#include <sstream>
#include <format>
#include <filesystem>

#include "Editor.h"

using namespace Yngin;

namespace fs = std::filesystem;

Editor::Editor() {
	fs::create_directory("temp");
	fs::create_directory("scenes");
	fs::create_directory("bin");

	fs::path cwd = fs::current_path();
	projectName = cwd.filename().string();

	GLFWmonitor* monitor = glfwGetPrimaryMonitor();
	const GLFWvidmode* mode = glfwGetVideoMode(monitor);

	ctx = createContext();

	{
		std::ifstream file("core.pak", std::ios::binary);
		if (file.is_open()) {
			std::ostringstream bytes(std::ios::binary);
			bytes << file.rdbuf();
			file.close();
			ctx->loadCorePak(bytes.str().c_str(), bytes.str().size());
			bytes.clear();
		}
	}

	{
		std::ifstream file("resources.pak", std::ios::binary);
		if (file.is_open()) {
			std::ostringstream bytes(std::ios::binary);
			bytes << file.rdbuf();
			file.close();
			ctx->loadResourcesPak(bytes.str().c_str(), bytes.str().size());
			bytes.clear();
		} else {
			ctx->getTexturesManager()->createTexture({
			.width = 2,
			.height = 2,
			.numCh = 1,
			.bytes = "\xff\x80\x80\xff"
				}, {
				.wrap = TEXTURE_WRAP::REPEAT,
				.filterMin = TEXTURE_FILTER::NEAREST,
				.filterMag = TEXTURE_FILTER::NEAREST
				}
			);
		}
	}

	Window* window = ctx->getWindow();
	window->setSize(glm::ivec2(1280, 720));
	window->setPosition(glm::ivec2((mode->width - 1280) / 2, (mode->height - 720) / 2));
	window->setTitle("Yngin Editor");

	ctx->meta.setMeta("#IsEditor", 1);


	ctx->setMaxFPS(0);

	if (ctx == nullptr || ctx->getStatus() != CONTEXT_STATUS::WAITING_FOR_READY) {
		printf("Failed to create context\n");
		return;
	}

	activeScene = ctx->getScenesManager()->createScene();

	{
		std::ifstream file("scenes/scene0.pak", std::ios::binary);
		if (file.is_open()) {
			std::ostringstream bytes(std::ios::binary);
			bytes << file.rdbuf();
			file.close();
			activeScene = ctx->getScenesManager()->createScene(bytes.str().c_str(), bytes.str().size(), 0, true);
			bytes.clear();
		} else {
			activeScene = ctx->getScenesManager()->createScene(0, true);

			Model* cubeModel = ctx->getModelsManager()->createModel(cubeModelData);

			GameObject* defaultCube = activeScene->getGameObjectsManager()->getRootGameObject()->createChild();
			Components::Mesh* defaultCubeMesh = defaultCube->createComponent<Components::Mesh>();
			defaultCubeMesh->setModel(cubeModel);
			defaultCubeMesh->setTexture(1);
			defaultCube->meta.setMeta("Editor.Name", "Cube");

			Texture* skyboxTex = ctx->getTexturesManager()->createTexture({
				.width = 1,
				.height = 1,
				.numCh = 3,
				.bytes = "\x4E\x4E\xFB",
				}, {
				.wrap = TEXTURE_WRAP::CLAMP,
				.filterMin = TEXTURE_FILTER::NEAREST,
				.filterMag = TEXTURE_FILTER::NEAREST,
				}
				);

			activeScene->setSkyboxTexture(skyboxTex);

			editorCamera = activeScene->getCamerasManager()->getCamera(0);
			editorCamera->setPosition(glm::vec3(2.0f));
			editorCamera->lookAt(glm::vec3());
		}
	}
	activeScene->activate();

	editorCamera = activeScene->getCamerasManager()->getCamera(0);

	ctx->getPhysicsEngine()->setSimulationEnabled(false);
	ctx->getRenderer()->setLightingEnabled(false);

	setupViewerScene();

	{
		std::ifstream file("scripts_editor.pak", std::ios::binary);
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
				}
			}

			file.close();
		}
	}

	lastSaved = ctx->getTime();

	ctx->ready();


	// initialize ImGui
	ImGui::CreateContext();
	ImGui_ImplGlfw_InitForOpenGL(ctx->getWindow()->getGLFWwindow(), true);
	ImGui_ImplOpenGL3_Init("#version 460");

	ImGuiStyle& style = ImGui::GetStyle();
	style.FrameRounding = 5.0f;

	ImGuiIO& io = ImGui::GetIO();
	io.IniFilename = nullptr;

	scriptEditor.SetLanguageDefinition(TextEditor::LanguageDefinition::Lua());
	scriptEditor.SetShowWhitespaces(false);
}

Editor::~Editor() {
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();

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
		if (c->getId() == 0) continue;
		ctx->getTexturesManager()->deleteTexture(c->getId());
	}

	for (auto& c : ctx->getGlobalUIManager()->getElements()) {
		if (c->getId() == 0) continue;
		ctx->getGlobalUIManager()->deleteElement(c);
	}

	for (auto& c : ctx->getScriptsManager()->getScripts()) {
		ctx->getScriptsManager()->deleteScript(c);
	}
}

void Editor::setupViewerScene() {
	viewerScene = ctx->getScenesManager()->createScene();
	viewerScene->meta.setMeta("#NoExport", 1);
	viewerObject = viewerScene->getGameObjectsManager()->getRootGameObject()->createChild();
	viewerObject->createComponent<Components::Mesh>();
	viewerObject->createComponent<Components::Light>();
	viewerScene->getCamerasManager()->getCamera(0)->setPosition(glm::vec3(1.0f));
	viewerScene->getCamerasManager()->getCamera(0)->lookAt(glm::vec3());

	viewerImage = viewerScene->getUIManager()->getRootElement()->createChild<UI::Image>();
}

void Editor::saveProject() {
	if (running) {
		printf("[Yngin Editor] Cannot save while the game is running!\n");
		return;
	}

	lastSaved = ctx->getTime();

	{
		std::ofstream file("core.pak", std::ios::binary);
		if (file.is_open()) {
			std::vector<char> bytes = ctx->generateCorePak();
			file.write(reinterpret_cast<const char*>(bytes.data()), bytes.size());
			file.close();
		}
	}

	{
		std::ofstream file("resources.pak", std::ios::binary);
		if (file.is_open()) {
			std::vector<char> bytes = ctx->generateResourcesPak();
			file.write(reinterpret_cast<const char*>(bytes.data()), bytes.size());
			file.close();
		}
	}

	{
		std::ofstream file("scripts_editor.pak", std::ios::binary);
		if (file.is_open()) {
			ScriptFileHeader header{};
			strcpy_s(header.magic, 19, "YNGINEDITORSCRIPTS");
			header.version = 0;
			header.scriptsCount = (uint32_t)scripts.size();
			file.write(reinterpret_cast<const char*>(&header), sizeof(ScriptFileHeader));

			for (auto& [id, script] : scripts) {
				ScriptInfo info{};
				info.id = id;
				info.nameSize = script.name.length();
				strcpy_s(info.name, sizeof(info.name), script.name.c_str());
				info.scene = script.scene;
				info.scriptSize = script.code.length();
				file.write(reinterpret_cast<const char*>(&info), sizeof(ScriptInfo));
				file << script.name;
				file << script.code;
			}
			file.close();
		}
	}

	{
		std::ofstream file("scenes/scene0.pak", std::ios::binary);
		if (file.is_open()) {
			std::vector<char> bytes = activeScene->generatePak();
			file.write(reinterpret_cast<const char*>(bytes.data()), bytes.size());
			file.close();
		}
	}

	printf("[Yngin Editor] Saved Project\n");
}

void Editor::exportGame() {
	if (running) {
		printf("[Yngin Editor] Cannot export while the game is running!\n");
		return;
	}
	setupPreviousGameState();

	loadScripts();

	Script* script = ctx->getScriptsManager()->createScript(std::format(R"LUA(
if Yngin.Context.meta:getMetaInt("#IsEditor", 0) == 1 then
	return
end

Yngin.Window:setTitle("{}")
Yngin.Window:setSize(IVec2.new({}, {}))
Yngin.Window:setFullscreen({})

function onReady()
	Yngin.ScenesManager:setActive({})
	
	Yngin.ScriptsManager:deleteScript(Script.ID)
end
)LUA", gameSettings.name, gameSettings.windowWidth, gameSettings.windowHeight, gameSettings.fullscreen, 0).c_str());

	{
		std::ofstream file("bin/game.pak", std::ios::binary);
		if (file) {
			std::vector<char> bytes = ctx->generateGamePak();
			file.write(reinterpret_cast<const char*>(bytes.data()), bytes.size());
			file.close();
		}
	}

	loadPreviousGameState();
}

void Editor::togglePlayMode() {
	running = !running;

	if (running) {
		setupPreviousGameState();
		ctx->meta.setMeta("#IsPlaying", 1);
		loadScripts();
	} else {
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

		ctx->loadGamePak(gameBytes.str().c_str(), gameBytes.str().size());
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
	ImGuiIO& io = ImGui::GetIO();

	Window* window = ctx->getWindow();

	glm::ivec2 windowSize = window->getSize();
	if (!viewingObject) {
		handleCameraMovement(editorCamera);
	} else {
		handleCameraMovement(viewerScene->getCamerasManager()->getCamera(0));
	}

	InputSystem* input = ctx->getInputSystem();

	if (input->isKeyJustPressed(KEY::F5)) {
		togglePlayMode();
	}

	//if (input->isKeyJustPressed(Yngin::KEY::F11) || (input->isKeyPressed(Yngin::KEY::RALT) && input->isKeyJustPressed(Yngin::KEY::ENTER))) {
		//window->setFullscreen(!window->isFullscreen());
	//}

	ctx->update(false);
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

				if (delta.x != 0 || delta.y != 0) window->minimize();

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
			if (ImGui::MenuItem("Save", "Ctrl+S")) {
				saveProject();
			}
			ImGui::Separator();
			if (ImGui::MenuItem("Exit", "Alt+F4")) {
				ctx->close();
			}
			ImGui::EndMenu();
		}

		if (ImGui::BeginMenu("Edit")) {
			bool enabled = explorerSelection.first == EXPLORER_SELECTION_TYPE::SCRIPT;
			if (ImGui::MenuItem("Undo", "Ctrl+Z", false, enabled && scriptEditor.CanUndo())) {
				scriptEditor.Undo();
			}
			if (ImGui::MenuItem("Redo", "Ctrl+Y", false, enabled && scriptEditor.CanRedo())) {
				scriptEditor.Redo();
			}
			ImGui::Separator();
			if (ImGui::MenuItem("Cut", "Ctrl+X", false, enabled)) {
				scriptEditor.Cut();
			}
			if (ImGui::MenuItem("Copy", "Ctrl+C", false, enabled)) {
				scriptEditor.Copy();
			}
			if (ImGui::MenuItem("Paste", "Ctrl+V", false, enabled)) {
				scriptEditor.Paste();
			}
			if (ImGui::MenuItem("Delete", "Del", false, enabled)) {
				scriptEditor.Delete();
			}
			ImGui::Separator();
			if (ImGui::MenuItem("Select All", "Ctrl+A", false, enabled)) {
				scriptEditor.SelectAll();
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

		if (ImGui::BeginMenu("Help")) {
			if (ImGui::MenuItem("GitHub Wiki")) {
				system("start https://github.com/YusufYaser/Yngin/wiki");
			}
			ImGui::EndMenu();
		}

		if (window->isFullscreen() || !window->hasTitleBar()) {
			static bool wasClickedLastFrame = false;
			if (!wasClickedLastFrame) {
				ImGui::SameLine(window->getSize().x - 55.0f * (window->isFullscreen() ? 1.0f : 1.5f));
				if (ImGui::BeginMenu("-")) {
					wasClickedLastFrame = true;
					if (window->isFullscreen()) {
						window->setFullscreen(false);
					} else {
						window->minimize();
					}
					ImGui::EndMenu();
				}

				if (!window->isFullscreen()) {
					if (ImGui::BeginMenu("O")) {
						wasClickedLastFrame = true;
						window->maximize();
						ImGui::EndMenu();
					}
				}

				ImGui::PushStyleColor(ImGuiCol_HeaderHovered, ImVec4(1, 0, 0, 1));
				if (ImGui::BeginMenu("X")) {
					wasClickedLastFrame = true;
					ctx->close();
					ImGui::EndMenu();
				}
				ImGui::PopStyleColor();
			} else {
				wasClickedLastFrame = false;
			}
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

	ImGui::SetNextWindowPos(ImVec2(255.0f, 25.0f));
	ImGui::SetNextWindowSize(ImVec2(125.0f, 50.0f));
	ImGui::Begin("Play Mode", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar);
	ImGui::PushStyleColor(ImGuiCol_Button, running ? ImVec4(.5f, 0, 0, 1) : ImVec4(0, .5f, 0, 1));
	if (explorerSelection.first != EXPLORER_SELECTION_TYPE::SCRIPT) {
		if (!viewingObject) {
			ImGui::Text("Scene Viewer");
			if (ImGui::Button(((running ? "Stop" : "Start") + std::string("##TogglePlayMode")).c_str())) {
				togglePlayMode();
			}
		} else {
			ImGui::Text("Resource Viewer");
		}
	}
	ImGui::PopStyleColor();
	ImGui::End();
	ctx->getPhysicsEngine()->setSimulationEnabled(running);
	ctx->getRenderer()->setLightingEnabled(running);

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

	if (viewingObject) {
		viewerScene->activate();
	} else {
		activeScene->activate();
	}

	glm::ivec2 viewportSize = ctx->getViewportSize();

	ImGui::SetNextWindowPos(ImVec2(250, windowSize.y - 300.0f));
	ImGui::SetNextWindowSize(ImVec2(windowSize.x - 250 - 300.0f, 300.0f));
	ImGui::Begin("Information", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar);
	ImGui::Text("FPS: %.1f", 1 / ctx->getDeltaTime());
	ImGui::Text("Viewport Size: %ix%i", viewportSize.x, viewportSize.y);
	ImGui::Text("%i GameObjects", activeScene->getGameObjectsManager()->getGameObjectsCount());
	ImGui::Text("%i UI Elements", activeScene->getUIManager()->getElementsCount());
	ImGui::Text("%i Textures", ctx->getTexturesManager()->getTexturesCount());
	ImGui::Text("%i Models", ctx->getModelsManager()->getModelsCount());
	ImGui::Text("Position: %f %f %f", editorCamera->getPosition().x, editorCamera->getPosition().y, editorCamera->getPosition().z);
	ImGui::End();

	glm::vec2 viewPos = { 250.0f, menubarHeight };
	glm::vec2 viewSize = { windowSize.x - 250.0f - 300.0f, windowSize.y - menubarHeight - 300.0f };
	ctx->forceViewport(viewPos, viewSize);
	if (explorerSelection.first == EXPLORER_SELECTION_TYPE::SCRIPT) {
		ctx->forceViewport({ -1, -1 }, { 1, 1 });

		ImGui::SetNextWindowPos({ viewPos.x, viewPos.y });
		ImGui::SetNextWindowSize({ viewSize.x, viewSize.y });
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));

		static int prevScriptId = -1;
		uint32_t scriptId = explorerSelection.second;

		auto it = scripts.find(scriptId);
		if (it != scripts.end()) {
			EditorScript& script = it->second;

			std::string title = script.name;

			ImGui::Begin(title.append("###Script Editor").c_str(), nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse);
			{

				if (scriptId != prevScriptId) {
					prevScriptId = scriptId;
					scriptEditor.SetText(script.code.c_str());
				}

				scriptEditor.SetReadOnly(running);
				scriptEditor.Render("Script Editor Code");
				if (scriptEditor.IsTextChanged()) {
					std::string codeStr = scriptEditor.GetText();
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
}
