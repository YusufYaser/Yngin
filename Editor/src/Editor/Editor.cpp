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
			.title = "Yngin Editor"
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

	Texture* gridTex = ctx->getTexturesManager()->createTexture({
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

	editorCamera = activeScene->getCamerasManager()->getCamera(0);

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
	Model* model = ctx->getModelsManager()->createModel(square);

	ctx->getPhysicsEngine()->setSimulationDistance(0);


	// initialize ImGui
	ImGui::CreateContext();
	ImGui_ImplGlfw_InitForOpenGL(ctx->getWindow()->getGLFWwindow(), true);
	ImGui_ImplOpenGL3_Init("#version 460");

	ImGuiStyle& style = ImGui::GetStyle();
	style.FrameRounding = 5.0f;
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

	glm::ivec2 windowSize = ctx->getWindow()->getSize();
	handleCameraMovement(editorCamera);

	InputSystem* input = ctx->getInputSystem();

	if (input->isKeyJustPressed(Yngin::KEY::F11) || (input->isKeyPressed(Yngin::KEY::RALT) && input->isKeyJustPressed(Yngin::KEY::ENTER))) {
		ctx->getWindow()->setFullscreen(!ctx->getWindow()->isFullscreen());
	}

	if (input->isKeyJustPressed(Yngin::KEY::SPACE)) {
		ctx->getPhysicsEngine()->setSimulationDistance(ctx->getPhysicsEngine()->getSimulationDistance() == 0.0f ? 256.0f : 0.0f);
	}

	ctx->update(false);
	io.DisplaySize = ImVec2((float)windowSize.x, (float)windowSize.y);

	ImGui_ImplOpenGL3_NewFrame();
	ImGui::NewFrame();

	float menubarHeight = 0;

	if (ImGui::BeginMainMenuBar()) {
		if (ctx->getWindow()->isFullscreen()) {
			ImGui::Text(ctx->getWindow()->getTitle());
			ImGui::Separator();
		}

		if (ImGui::BeginMenu("File")) {
			ImGui::MenuItem("Example Project", 0, false, false);
			ImGui::Separator();
			if (ImGui::MenuItem("Exit", "Alt+F4")) {
				return;
			}
			ImGui::EndMenu();
		}

		if (ImGui::BeginMenu("Help")) {
			if (ImGui::MenuItem("GitHub Wiki")) {
				system("start https://github.com/YusufYaser/Yngin/wiki");
			}
			ImGui::EndMenu();
		}

		if (ctx->getWindow()->isFullscreen()) {
			ImGui::SameLine(ctx->getWindow()->getSize().x - 55.0f);
			if (ImGui::BeginMenu("-")) {
				ctx->getWindow()->setFullscreen(false);
				ImGui::EndMenu();
			}
			ImGui::PushStyleColor(ImGuiCol_HeaderHovered, ImVec4(1, 0, 0, 1));
			if (ImGui::BeginMenu("X")) {
				ImGui::EndMenu();
				return;
			}
			ImGui::PopStyleColor();
		}

		menubarHeight = ImGui::GetFrameHeight();

		ImGui::EndMainMenuBar();
	}

	ImGuiViewport* viewport = ImGui::GetMainViewport();
	showExplorer();

	if (ImGui::BeginViewportSideBar("##Properties", viewport, ImGuiDir_Right, 300.0f, ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoNavFocus)) {
		if (explorerSelection.first == EXPLORER_SELECTION_TYPE::GAMEOBJECT) {
			GameObject* obj = activeScene->getGameObjectsManager()->getGameObject(explorerSelection.second);
			if (obj) {
				gameObjectProps(obj);
			}
		} else if (explorerSelection.first == EXPLORER_SELECTION_TYPE::UIELEMENT) {
			UI::UIElement* element = activeScene->getUIManager()->getElement(explorerSelection.second);
			if (element) {
				if (element->getId() != 0) {
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
		}
		ImGui::End();
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
