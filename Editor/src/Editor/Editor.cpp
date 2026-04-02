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

#include "Editor.h"

using namespace Yngin;

Editor::Editor() {
	GLFWmonitor* monitor = glfwGetPrimaryMonitor();
	const GLFWvidmode* mode = glfwGetVideoMode(monitor);

	ctx = createContext({
		.windowSettings = {
			.size = glm::ivec2(1280, 720),
			.position = glm::ivec2((mode->width - 1280) / 2, (mode->height - 720) / 2),
			.title = "Yngin Editor",
			.hasTitleBar = true
			}
		});


	ctx->setMaxFPS(0);

	if (ctx == nullptr || ctx->getStatus() != CONTEXT_STATUS::WAITING_FOR_READY) {
		printf("Failed to create context\n");
		return;
	}

	activeScene = ctx->getScenesManager()->createScene();
	activeScene->activate();

	editorCamera = activeScene->getCamerasManager()->getCamera(0);
	editorCamera->setPosition(glm::vec3(2.0f));
	editorCamera->lookAt(glm::vec3());

	TexturesManager* texturesManager = ctx->getTexturesManager();

	Texture* skyboxTex = ctx->getTexturesManager()->createTexture("assets/default_skybox.png", {
		.wrap = TEXTURE_WRAP::CLAMP,
		.filterMin = TEXTURE_FILTER::NEAREST,
		.filterMag = TEXTURE_FILTER::NEAREST,
		});
	activeScene->setSkyboxTexture(skyboxTex);

	gridTexture = ctx->getTexturesManager()->createTexture({
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

	cubeModel = ctx->getModelsManager()->createModel(cubeModelData);

	GameObject* defaultCube = activeScene->getGameObjectsManager()->getRootGameObject()->createChild();
	Components::Mesh* defaultCubeMesh = defaultCube->createComponent<Components::Mesh>();
	defaultCubeMesh->setModel(cubeModel);
	defaultCubeMesh->setTexture(gridTexture);
	defaultCube->meta.setMeta("Editor.Name", "Cube");

	ctx->getPhysicsEngine()->setSimulationEnabled(false);
	ctx->getRenderer()->setLightingEnabled(false);

	setupViewerScene();

	ctx->getScriptsManager()->createScript(R"LUA(
	-- Scene Activator
	
	function onReady()
		Yngin.ScenesManager:setActive(0)
	end
)LUA");

	ctx->ready();


	// initialize ImGui
	ImGui::CreateContext();
	ImGui_ImplGlfw_InitForOpenGL(ctx->getWindow()->getGLFWwindow(), true);
	ImGui_ImplOpenGL3_Init("#version 460");

	ImGuiStyle& style = ImGui::GetStyle();
	style.FrameRounding = 5.0f;

	ImGuiIO& io = ImGui::GetIO();
	io.IniFilename = nullptr;
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

void Editor::exportGame() {
	setupPreviousGameState();

	Script* script = ctx->getScriptsManager()->createScript(std::format(R"LUA(
	Yngin.Window:setTitle("{}")
	Yngin.Window:setSize(IVec2.new({}, {}))
	Yngin.Window:setFullscreen({})
	
	Yngin.ScriptsManager:deleteScript(Script.ID)
)LUA", gameSettings.name, gameSettings.windowWidth, gameSettings.windowHeight, gameSettings.fullscreen).c_str());

	{
		std::ofstream file("game.pak", std::ios::binary);
		if (file) {
			std::vector<char> bytes = ctx->generateGamePak();
			file.write(reinterpret_cast<const char*>(bytes.data()), bytes.size());
			file.close();
		}
	}

	loadPreviousGameState();
}

void Editor::setupPreviousGameState() {
	std::ofstream file("previous_game_state.pak", std::ios::binary);
	if (file) {
		std::vector<char> bytes = ctx->generateGamePak();
		file.write(reinterpret_cast<const char*>(bytes.data()), bytes.size());
		file.close();
	}
}

void Editor::loadPreviousGameState() {
	std::ifstream gamePak("previous_game_state.pak", std::ios::binary);
	if (gamePak.is_open()) {
		uint32_t activeSceneId = activeScene->getId();
		uint32_t editorCameraId = editorCamera->getId();
		uint32_t gridTextureId = gridTexture->getId();
		uint32_t cubeModelId = cubeModel->getId();

		resetContext();

		std::ostringstream gameBytes(std::ios::binary);
		gameBytes << gamePak.rdbuf();
		gamePak.close();

		std::remove("previous_game_state.pak");

		ctx->loadGamePak(gameBytes.str().c_str(), gameBytes.str().size());
		gameBytes.clear();

		activeScene = ctx->getScenesManager()->getScene(activeSceneId);
		editorCamera = activeScene->getCamerasManager()->getCamera(editorCameraId);
		gridTexture = ctx->getTexturesManager()->getTexture(gridTextureId);
		cubeModel = ctx->getModelsManager()->getModel(cubeModelId);

		activeScene->activate();

		setupViewerScene();
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
		exportGame();
	}

	if (input->isKeyJustPressed(Yngin::KEY::F11) || (input->isKeyPressed(Yngin::KEY::RALT) && input->isKeyJustPressed(Yngin::KEY::ENTER))) {
		window->setFullscreen(!window->isFullscreen());
	}

	ctx->update(false);
	io.DisplaySize = ImVec2((float)windowSize.x, (float)windowSize.y);

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
			ImGui::MenuItem("Example Project", 0, false, false);
			ImGui::Separator();
			if (ImGui::MenuItem("Exit", "Alt+F4")) {
				ctx->close();
			}
			ImGui::EndMenu();
		}

		if (ImGui::BeginMenu("Play")) {
			if (ImGui::MenuItem(running ? "Stop Play Mode" : "Start Play Mode")) {
				running = !running;

				if (running) {
					setupPreviousGameState();
				} else {
					loadPreviousGameState();
				}
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

		menubarHeight = ImGui::GetFrameHeight();

		ImGui::EndMainMenuBar();
	}

	ImGui::SetNextWindowPos(ImVec2(255.0f, 25.0f));
	ImGui::SetNextWindowSize(ImVec2(125.0f, 50.0f));
	ImGui::Begin("Play Mode", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar);
	ImGui::PushStyleColor(ImGuiCol_Button, running ? ImVec4(.5f, 0, 0, 1) : ImVec4(0, .5f, 0, 1));
	if (!viewingObject) {
		ImGui::Text("Scene Viewer");
		if (ImGui::Button(((running ? "Stop" : "Start") + std::string("##TogglePlayMode")).c_str())) {
			running = !running;

			if (running) {
				setupPreviousGameState();
			} else {
				loadPreviousGameState();
			}
		}
	} else {
		ImGui::Text("Resource Viewer");
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

		case EXPLORER_SELECTION_TYPE::GAMEOBJECT:
			showGameObjectProps(explorerSelection.second);
			break;

		case EXPLORER_SELECTION_TYPE::UIELEMENT:
		{
			UI::UIElement* element = activeScene->getUIManager()->getElement(explorerSelection.second);
			if (element) {
				if (element->getId() != -1) {
					ImGui::Text("Properties (UI Element #%i)", element->getId());
					ImGui::Separator();

					if (ImGui::Button("Delete")) {
						activeScene->getUIManager()->deleteElement(element);
						explorerSelection = {};
					}
				} else {
					ImGui::Text("Root UI Element");
				}
			}
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
	ImGui::Begin("Information", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse);
	ImGui::Text("FPS: %.1f", 1 / ctx->getDeltaTime());
	ImGui::Text("Viewport Size: %ix%i", viewportSize.x, viewportSize.y);
	ImGui::Text("%i GameObjects", activeScene->getGameObjectsManager()->getGameObjectsCount());
	ImGui::Text("%i UI Elements", activeScene->getUIManager()->getElementsCount());
	ImGui::Text("%i Textures", ctx->getTexturesManager()->getTexturesCount());
	ImGui::Text("%i Models", ctx->getModelsManager()->getModelsCount());
	ImGui::Text("Position: %f %f %f", editorCamera->getPosition().x, editorCamera->getPosition().y, editorCamera->getPosition().z);
	ImGui::End();

	ctx->forceViewport({ 250.0f, menubarHeight }, { windowSize.x - 250.0f - 300.0f, windowSize.y - menubarHeight - 300.0f });

	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

	ctx->swapBuffers();
}
