#include <stdio.h>
#include <Yngin/Yngin.h>
#include <ImGui/imgui.h>
#include <ImGui/imgui_impl_opengl3.h>
#include <ImGui/imgui_impl_glfw.h>
#include <ImGui/imgui_internal.h>
#include <GLFW/glfw3.h>

#include "Camera.h"
#include "Explorer.h"

using namespace Yngin;

int main() {
	initializeYngin();

	if (!isYnginInitialized()) {
		printf("Failed to initialize Yngin\n");
		return 1;
	}

	// initialize Yngin

	GLFWmonitor* monitor = glfwGetPrimaryMonitor();
	const GLFWvidmode* mode = glfwGetVideoMode(monitor);

	Context* ctx = createContext({
		.windowSettings = {
			.size = glm::ivec2(1280, 720),
			.position = glm::ivec2((mode->width - 1280) / 2, (mode->height - 720) / 2),
			.title = "Yngin Editor"
			}
		});

	ctx->setMaxFPS(0);

	if (ctx == nullptr || ctx->getStatus() != CONTEXT_STATUS::RUNNING) {
		printf("Failed to create context\n");
		return 1;
	}

	Scene* scene = ctx->getScenesManager()->createScene();
	scene->activate();

	TexturesManager* texturesManager = ctx->getTexturesManager();

	Texture* skyboxTex = ctx->getTexturesManager()->createTexture("assets/default_skybox.png", {
		.wrap = TEXTURE_WRAP::CLAMP,
		.filterMin = TEXTURE_FILTER::NEAREST,
		.filterMag = TEXTURE_FILTER::NEAREST,
		});
	scene->setSkyboxTexture(skyboxTex);

	InputSystem* input = ctx->getInputSystem();
	Camera* editorCamera = scene->getCamerasManager()->getCamera(0);

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

	// create example game objcets
	for (int a = 0; a < 5; a++) {
		GameObject* objA = scene->getGameObjectsManager()->getRootGameObject()->createChild();
		objA->createComponent<Components::Mesh>()->setModel(model);
		for (int b = 0; b < 5; b++) {
			GameObject* objB = objA->createChild();
			objB->createComponent<Components::Mesh>()->setModel(model);
			for (int c = 0; c < 5; c++) {
				objB->createChild()->createComponent<Components::Mesh>()->setModel(model);
			}
		}
	}
	for (int a = 0; a < 5; a++) {
		UI::UIElement* objA = scene->getUIManager()->getRootElement()->createChild();
		for (int b = 0; b < 5; b++) {
			UI::UIElement* objB = objA->createChild();
			for (int c = 0; c < 5; c++) {
				objB->createChild();
			}
		}
	}

	ctx->getPhysicsEngine()->setSimulationDistance(0);

	// initialize ImGui
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGui_ImplGlfw_InitForOpenGL(ctx->getWindow()->getGLFWwindow(), true);
	ImGui_ImplOpenGL3_Init("#version 460");

	ImGuiIO& io = ImGui::GetIO();

	float f = 0;

	std::pair<EXPLORER_SELECTION_TYPE, int> explorerSelection;

	glm::vec3 pos{};
	glm::vec3 rotation{};

	int meshModelInput = 0;
	int meshTexInput = 0;
	float lightIntensityInput = 0;
	float lightDistanceInput = 0;
	float lightColorInput[3] = {};
	float rigidBodyMassInput = 0.0f;

	while (ctx->getStatus() == CONTEXT_STATUS::RUNNING) {
		glm::ivec2 windowSize = ctx->getWindow()->getSize();
		handleCameraMovement(editorCamera);

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
					break;
				}
				ImGui::EndMenu();
			}

			if (ImGui::BeginMenu("Help")) {
				if (ImGui::MenuItem("GitHub")) {
					system("start https://github.com/YusufYaser/Yngin");
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
					break;
				}
				ImGui::PopStyleColor();
			}

			menubarHeight = ImGui::GetFrameHeight();

			ImGui::EndMainMenuBar();
		}

		ImGuiViewport* viewport = ImGui::GetMainViewport();
		auto newSelection = showExplorer(scene);
		if (newSelection.first != EXPLORER_SELECTION_TYPE::NONE) {
			explorerSelection = newSelection;
		}

		bool changingPos = false;
		bool changingRotation = false;

		if (ImGui::BeginViewportSideBar("##Properties", viewport, ImGuiDir_Right, 300.0f, ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoNavFocus)) {
			if (explorerSelection.first == EXPLORER_SELECTION_TYPE::GAMEOBJECT) {
				GameObject* obj = scene->getGameObjectsManager()->getGameObject(explorerSelection.second);
				if (obj) {
					if (obj->getId() != 0) {
						ImGui::Text("Properties (GameObject #%i)", obj->getId());
						ImGui::Separator();
						ImGui::Text("Position");
						ImGui::SameLine();
						if (ImGui::SmallButton("Move to Camera")) {
							obj->setPosition(editorCamera->getPosition());
						}
						ImGui::SetNextItemWidth(90.0f);
						changingPos |= ImGui::InputFloat("##PosX", &pos.x, 1.0f, 1.0f, "%.2f");
						ImGui::SameLine();
						ImGui::SetNextItemWidth(90.0f);
						changingPos |= ImGui::InputFloat("##PosY", &pos.y, 1.0f, 1.0f, "%.2f");
						ImGui::SameLine();
						ImGui::SetNextItemWidth(90.0f);
						changingPos |= ImGui::InputFloat("##PosZ", &pos.z, 1.0f, 1.0f, "%.2f");

						ImGui::Separator();
						ImGui::Text("Rotation");
						ImGui::SetNextItemWidth(90.0f);
						changingRotation |= ImGui::InputFloat("##RotationX", &rotation.x, 1.0f, 22.5f, "%.1f");
						ImGui::SameLine();
						ImGui::SetNextItemWidth(90.0f);
						changingRotation |= ImGui::InputFloat("##RotationY", &rotation.y, 1.0f, 22.5f, "%.1f");
						ImGui::SameLine();
						ImGui::SetNextItemWidth(90.0f);
						changingRotation |= ImGui::InputFloat("##RotationZ", &rotation.z, 1.0f, 22.5f, "%.1f");

						ImGui::Separator();

						ImGui::Text("Mesh Component");
						Components::Mesh* mesh = obj->getComponent<Components::Mesh>();
						if (mesh) {
							ImGui::SameLine();
							if (!ImGui::SmallButton("Delete##MeshComp")) {
								ImGui::Text("Model ID");
								ImGui::SameLine(100);
								if (ImGui::InputInt("##MeshModelID", &meshModelInput)) {
									mesh->setModel(meshModelInput);
								} else {
									meshModelInput = mesh->getModel();
								}
								ImGui::Text("Texture ID");
								ImGui::SameLine(100);
								if (ImGui::InputInt("##MeshTextureID", &meshTexInput)) {
									mesh->setTexture(meshTexInput);
								} else {
									meshTexInput = mesh->getTexture();
								}
							} else {
								obj->deleteComponent<Components::Mesh>();
							}
						} else {
							if (ImGui::Button("Create Component##Mesh", ImVec2(284.0f, 0))) {
								obj->createComponent<Components::Mesh>();
							}
						}

						ImGui::Separator();

						ImGui::Text("Light Component");
						Components::Light* light = obj->getComponent<Components::Light>();
						if (light) {
							ImGui::SameLine();
							if (!ImGui::SmallButton("Delete##LightComp")) {
								ImGui::Text("Intensity");
								ImGui::SameLine();
								if (ImGui::InputFloat("##LightIntensity", &lightIntensityInput)) {
									light->setIntensity(lightIntensityInput);
								} else {
									lightIntensityInput = light->getIntensity();
								}
								ImGui::Text("Distance");
								ImGui::SameLine();
								if (ImGui::InputFloat("##LightDistance", &lightDistanceInput)) {
									light->setDistance(lightDistanceInput);
								} else {
									lightDistanceInput = light->getDistance();
								}
								if (ImGui::ColorPicker3("Color", lightColorInput)) {
									light->setColor(glm::vec3(lightColorInput[0], lightColorInput[1], lightColorInput[2]));
								} else {
									lightColorInput[0] = light->getColor()[0];
									lightColorInput[1] = light->getColor()[1];
									lightColorInput[2] = light->getColor()[2];
								}
							} else {
								obj->deleteComponent<Components::Light>();
							}
						} else {
							if (ImGui::Button("Create Component##Light", ImVec2(284.0f, 0))) {
								obj->createComponent<Components::Light>();
							}
						}

						ImGui::Separator();

						ImGui::Text("RigidBody Component");
						Components::RigidBody* rigidBody = obj->getComponent<Components::RigidBody>();
						if (rigidBody) {
							ImGui::SameLine();
							if (!ImGui::SmallButton("Delete##RigidBodyComp")) {
								ImGui::Text("Mass");
								ImGui::SameLine();
								if (ImGui::InputFloat("##RigidBodyMass", &rigidBodyMassInput)) {
									rigidBody->setMass(rigidBodyMassInput);
								} else {
									rigidBodyMassInput = rigidBody->getMass();
								}
							} else {
								obj->deleteComponent<Components::RigidBody>();
							}
						} else {
							if (ImGui::Button("Create Component##RigidBody", ImVec2(284.0f, 0))) {
								obj->createComponent<Components::RigidBody>();
							}
						}

						ImGui::Separator();

						if (ImGui::Button("Create Child")) {
							explorerSelection = { EXPLORER_SELECTION_TYPE::GAMEOBJECT, obj->createChild()->getId() };
						}
						ImGui::SameLine();
						if (ImGui::Button("Teleport")) {
							editorCamera->setPosition(obj->getPosition());
						}

						if (ImGui::Button("Delete")) {
							scene->getGameObjectsManager()->deleteGameObject(obj);
							explorerSelection = {};
						}
					} else {
						ImGui::Text("Root Game Object");
						if (ImGui::Button("Create GameObject")) {
							explorerSelection = { EXPLORER_SELECTION_TYPE::GAMEOBJECT, obj->createChild()->getId() };
						}
					}
				}
			} else if (explorerSelection.first == EXPLORER_SELECTION_TYPE::UIELEMENT) {
				UI::UIElement* element = scene->getUIManager()->getElement(explorerSelection.second);
				if (element) {
					if (element->getId() != 0) {
						ImGui::Text("Properties (UI Element #%i)", element->getId());
						ImGui::Separator();

						if (ImGui::Button("Delete")) {
							scene->getUIManager()->deleteElement(element);
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
		ImGui::Text("%i GameObjects", scene->getGameObjectsManager()->getGameObjectsCount());
		ImGui::Text("%i UI Elements", scene->getUIManager()->getElementsCount());
		ImGui::Text("Position: %f %f %f", editorCamera->getPosition().x, editorCamera->getPosition().y, editorCamera->getPosition().z);
		ImGui::End();

		ctx->forceViewport({ 250.0f, menubarHeight }, { windowSize.x - 250.0f - 300.0f, windowSize.y - menubarHeight - 300.0f });

		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

		if (explorerSelection.first == EXPLORER_SELECTION_TYPE::GAMEOBJECT) {
			GameObject* obj = scene->getGameObjectsManager()->getGameObject(explorerSelection.second);
			if (obj) {
				if (changingPos) {
					obj->setPosition(pos);
				} else {
					pos = obj->getPosition();
				}
				if (changingRotation) {
					obj->setRotation(rotation * (3.1415f / 180));
				} else {
					rotation = obj->getRotation() * (180 / 3.1415f);
				}
			}
		}

		ctx->swapBuffers();
	}

	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();

	delete ctx;
	ctx = nullptr;

	Yngin::terminateYngin();

	return 0;
}
