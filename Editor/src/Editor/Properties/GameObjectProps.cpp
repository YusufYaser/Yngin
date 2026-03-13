#include "GameObjectProps.h"
#include <Yngin/Core/Scenes.h>
#include <Yngin/Components/Components.h>
#include <Yngin/Rendering/Cameras.h>
#include <ImGui/imgui.h>
#include "../Explorer.h"

using namespace Yngin;

extern Yngin::Scene* editorScene;

extern Camera* editorCamera;

extern std::pair<EXPLORER_SELECTION_TYPE, int> explorerSelection;

void gameObjectProps(Yngin::GameObject* obj) {
	glm::vec3 pos{};
	glm::vec3 rotation{};

	static int meshModelInput = 0;
	static int meshTexInput = 0;
	static float meshColorInput[3] = {};
	static float lightIntensityInput = 0;
	static float lightDistanceInput = 0;
	static float lightColorInput[3] = {};
	static float rigidBodyMassInput = 0.0f;

	bool changingPos = false;
	bool changingRotation = false;

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
				if (ImGui::ColorPicker3("Color##MeshColor", meshColorInput)) {
					mesh->setColor(glm::vec3(meshColorInput[0], meshColorInput[1], meshColorInput[2]));
				} else {
					meshColorInput[0] = mesh->getColor()[0];
					meshColorInput[1] = mesh->getColor()[1];
					meshColorInput[2] = mesh->getColor()[2];
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
				if (ImGui::ColorPicker3("Color##LightColor", lightColorInput)) {
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
			editorScene->getGameObjectsManager()->deleteGameObject(obj);
			explorerSelection = {};
		}
	} else {
		ImGui::Text("Root Game Object");
		if (ImGui::Button("Create GameObject", ImVec2(-1, 40))) {
			explorerSelection = { EXPLORER_SELECTION_TYPE::GAMEOBJECT, obj->createChild()->getId() };
		}
	}

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
