#include <stdio.h>
#include <Yngin/Yngin.h>
#include <ImGui/imgui.h>
#include <ImGui/imgui_impl_opengl3.h>
#include <ImGui/imgui_impl_glfw.h>
#include <ImGui/imgui_internal.h>
#include <GLFW/glfw3.h>

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

	if (ctx == nullptr || ctx->getStatus() != CONTEXT_STATUS::RUNNING) {
		printf("Failed to create context\n");
		return;
	}

	activeScene = ctx->getScenesManager()->createScene();
	activeScene->activate();

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

	editorCamera = activeScene->getCamerasManager()->createCamera();
	activeScene->getCamerasManager()->getCamera(0)->setWeight(0);
	editorCamera->setWeight(1.0f);

	const ModelData square = {
		{
			Vertex{glm::vec3(+0.5f, +0.5f, 0.0f), glm::vec2(1.0f, 0.0f), glm::vec3(0, 0, 1.0f)},
			Vertex{glm::vec3(-0.5f, +0.5f, 0.0f), glm::vec2(0.0f, 0.0f), glm::vec3(0, 0, 1.0f)},
			Vertex{glm::vec3(-0.5f, -0.5f, 0.0f), glm::vec2(0.0f, 1.0f), glm::vec3(0, 0, 1.0f)},
			Vertex{glm::vec3(+0.5f, -0.5f, 0.0f), glm::vec2(1.0f, 1.0f), glm::vec3(0, 0, 1.0f)},
		},

		{
			0, 1, 2, 0, 2, 3
		},

		MODEL_FRONT_FACE::NONE
	};
	squareModel = ctx->getModelsManager()->createModel(square);

	ctx->getPhysicsEngine()->setSimulationEnabled(false);
	ctx->getRenderer()->setLightingEnabled(false);

	viewerScene = ctx->getScenesManager()->createScene();
	viewerObject = viewerScene->getGameObjectsManager()->getRootGameObject()->createChild();
	viewerObject->createComponent<Components::Mesh>();
	viewerObject->createComponent<Components::Light>();
	viewerScene->getCamerasManager()->getCamera(0)->setPosition(glm::vec3(1.0f));
	viewerScene->getCamerasManager()->getCamera(0)->lookAt(glm::vec3());

	viewerImage = viewerScene->getUIManager()->getRootElement()->createChild<UI::Image>();


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
	ImGui::SetNextWindowSize(ImVec2(125.0f, 25.0f));
	ImGui::Begin("Simulation Control", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar);
	ImGui::PushStyleColor(ImGuiCol_Button, simulating ? ImVec4(.5f, 0, 0, 1) : ImVec4(0, .5f, 0, 1));
	if (!viewingObject) {
		if (ImGui::Button(((simulating ? "Stop" : "Start") + std::string("##ToggleSimulation")).c_str())) {
			simulating = !simulating;
		}
	} else {
		ImGui::Text("Resource Viewer");
	}
	ImGui::PopStyleColor();
	ImGui::End();
	ctx->getPhysicsEngine()->setSimulationEnabled(simulating);
	ctx->getRenderer()->setLightingEnabled(simulating);

	ImGuiViewport* viewport = ImGui::GetMainViewport();

	if (ImGui::BeginViewportSideBar("##Explorer", viewport, ImGuiDir_Left, 250.0f, 0)) {
		ImGui::Text("Project Explorer");
		ImGui::Separator();
		ImGui::BeginTabBar("Explorer Tabs");
		if (ImGui::BeginTabItem("Scene")) {
			showSceneExplorer();
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

	ImGui::SetNextWindowPos(ImVec2(250, windowSize.y - 300.0f));
	ImGui::SetNextWindowSize(ImVec2(windowSize.x - 250 - 300.0f, 300.0f));
	ImGui::Begin("Information", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse);
	ImGui::Text("FPS: %.1f", 1 / ctx->getDeltaTime());
	ImGui::Text("%i GameObjects", activeScene->getGameObjectsManager()->getGameObjectsCount());
	ImGui::Text("%i UI Elements", activeScene->getUIManager()->getElementsCount());
	ImGui::Text("Position: %f %f %f", editorCamera->getPosition().x, editorCamera->getPosition().y, editorCamera->getPosition().z);
	ImGui::End();

	ctx->forceViewport({ 250.0f, menubarHeight }, { windowSize.x - 250.0f - 300.0f, windowSize.y - menubarHeight - 300.0f });

	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

	ctx->swapBuffers();
}
