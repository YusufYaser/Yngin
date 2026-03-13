#include <Yngin/Core/Scenes.h>
#include <Yngin/Components/Components.h>
#include <Yngin/Rendering/Cameras.h>
#include <ImGui/imgui.h>
#include "../Editor.h"
#include <string>

using namespace Yngin;

void Editor::gameObjectProps(Yngin::GameObject* obj) {
	if (obj->getId() != 0) {
		ImGui::Text("Properties (GameObject #%i)", obj->getId());
		ImGui::Separator();
		ImGui::Text("Position");
		ImGui::SameLine();
		if (ImGui::SmallButton("Move to Camera")) {
			obj->setPosition(editorCamera->getPosition());
		}
		{
			glm::vec3 pos = obj->getPosition();
			static glm::vec3 v = {};

			ImGui::SetNextItemWidth(90.0f);
			if (ImGui::InputFloat("##PosX", &v.x, 1.0f, 1.0f, "%.2f")) {
				pos.x = v.x;
			} else {
				v.x = pos.x;
			}
			ImGui::SameLine();
			ImGui::SetNextItemWidth(90.0f);
			if (ImGui::InputFloat("##PosY", &v.y, 1.0f, 1.0f, "%.2f")) {
				pos.y = v.y;
			} else {
				v.y = pos.y;
			}
			ImGui::SameLine();
			ImGui::SetNextItemWidth(90.0f);
			if (ImGui::InputFloat("##PosZ", &v.z, 1.0f, 1.0f, "%.2f")) {
				pos.z = v.z;
			} else {
				v.z = pos.z;
			}
			obj->setPosition(pos);
		}

		ImGui::Separator();
		ImGui::Text("Rotation");
		{
			glm::vec3 rot = obj->getRotation();
			static glm::vec3 v = {};

			const float degToRad = (3.14159265359f / 180);

			ImGui::SetNextItemWidth(90.0f);
			if (ImGui::InputFloat("##RotationX", &v.x, 1.0f, 22.5f, "%.1f")) {
				rot.x = v.x * degToRad;
			} else {
				v.x = rot.x / degToRad;
			}
			ImGui::SameLine();
			ImGui::SetNextItemWidth(90.0f);
			if (ImGui::InputFloat("##RotationY", &v.y, 1.0f, 22.5f, "%.1f")) {
				rot.y = v.y * degToRad;
			} else {
				v.y = rot.y / degToRad;
			}
			ImGui::SameLine();
			ImGui::SetNextItemWidth(90.0f);
			if (ImGui::InputFloat("##RotationZ", &v.z, 1.0f, 22.5f, "%.1f")) {
				rot.z = v.z * degToRad;
			} else {
				v.z = rot.z / degToRad;
			}
			obj->setRotation(rot);
		}

		ImGui::Separator();

		std::vector<const char*> componentsToCreate = { "Select Component" };

		Components::Mesh* mesh = obj->getComponent<Components::Mesh>();
		if (mesh) {
			ImGui::Text("Mesh Component");
			ImGui::SameLine();
			if (!ImGui::SmallButton("Delete##MeshComp")) {
				ImGui::Text("Model ID");
				ImGui::SameLine(100);
				{
					static int v = 0;
					if (ImGui::InputInt("##MeshModelID", &v)) {
						mesh->setModel(v);
					} else {
						v = mesh->getModel();
					}
				}
				ImGui::Text("Texture ID");
				ImGui::SameLine(100);
				{
					static int v = 0;
					if (ImGui::InputInt("##MeshTextureID", &v)) {
						mesh->setTexture(v);
					} else {
						v = mesh->getTexture();
					}
				}
				{
					static float v[3] = {};
					ImGui::Text("Color");
					ImGui::SameLine();
					ImGui::ColorButton("MeshColorPreview", ImVec4(v[0], v[1], v[2], 1), ImGuiColorEditFlags_NoDragDrop, ImVec2(50, ImGui::GetFrameHeight()));
					ImGui::SameLine();
					if (ImGui::Button("Change Color##MeshColorButton", ImVec2(-1, 0))) {
						ImGui::OpenPopup("Mesh Color Picker");
					}

					if (ImGui::BeginPopup("Mesh Color Picker")) {
						if (ImGui::ColorPicker3("Color##MeshColor", v)) {
							mesh->setColor(glm::vec3(v[0], v[1], v[2]));
						}
						ImGui::EndPopup();
					} else {
						v[0] = mesh->getColor()[0];
						v[1] = mesh->getColor()[1];
						v[2] = mesh->getColor()[2];
					}
				}
			} else {
				obj->deleteComponent<Components::Mesh>();
			}
			ImGui::Separator();
		} else {
			componentsToCreate.push_back("Mesh");
		}


		Components::Light* light = obj->getComponent<Components::Light>();
		if (light) {
			ImGui::Text("Light Component");
			ImGui::SameLine();
			if (!ImGui::SmallButton("Delete##LightComp")) {
				ImGui::Text("Intensity");
				ImGui::SameLine(100);
				{
					static float v = 0;
					if (ImGui::InputFloat("##LightIntensity", &v, 0.05f, 0.1f)) {
						if (v > 1) v = 1;
						light->setIntensity(v);
					} else {
						v = light->getIntensity();
					}
				}
				ImGui::Text("Distance");
				ImGui::SameLine(100);
				{
					static float v = 0;
					if (ImGui::InputFloat("##LightDistance", &v, 1.0f, 2.0f)) {
						if (v < 0) v = 0;
						light->setDistance(v);
					} else {
						v = light->getDistance();
					}
				}
				{
					static float v[3] = {};
					ImGui::Text("Color");
					ImGui::SameLine();
					ImGui::ColorButton("LightColorPreview", ImVec4(v[0], v[1], v[2], 1), ImGuiColorEditFlags_NoDragDrop, ImVec2(50, ImGui::GetFrameHeight()));
					ImGui::SameLine();
					if (ImGui::Button("Change Color##LightColorButton", ImVec2(-1, 0))) {
						ImGui::OpenPopup("Light Color Picker");
					}

					if (ImGui::BeginPopup("Light Color Picker")) {
						if (ImGui::ColorPicker3("Color##LightColor", v)) {
							light->setColor(glm::vec3(v[0], v[1], v[2]));
						}
						ImGui::EndPopup();
					} else {
						v[0] = light->getColor()[0];
						v[1] = light->getColor()[1];
						v[2] = light->getColor()[2];
					}
				}
			} else {
				obj->deleteComponent<Components::Light>();
			}
			ImGui::Separator();
		} else {
			componentsToCreate.push_back("Light");
		}


		Components::RigidBody* rigidBody = obj->getComponent<Components::RigidBody>();
		if (rigidBody) {
			ImGui::Text("RigidBody Component");
			ImGui::SameLine();
			if (!ImGui::SmallButton("Delete##RigidBodyComp")) {
				ImGui::Text("Mass");
				ImGui::SameLine(100);
				{
					static float v = 0;
					if (ImGui::InputFloat("##RigidBodyMass", &v)) {
						rigidBody->setMass(v);
					} else {
						v = rigidBody->getMass();
					}
				}
				ImGui::Text("Elasticity");
				ImGui::SameLine(100);
				{
					static float v = 0;
					if (ImGui::InputFloat("##RigidBodyElasticity", &v)) {
						if (v > 1) v = 1;
						if (v < 1) v = 0;
						rigidBody->setElasticity(v);
					} else {
						v = rigidBody->getElasticity();
					}
				}
			} else {
				obj->deleteComponent<Components::RigidBody>();
			}
			ImGui::Separator();
		} else {
			componentsToCreate.push_back("RigidBody");
		}


		{
			static int selected = 0;
			if (componentsToCreate.size() != 1) {
				ImGui::Text("Create Component");
				ImGui::Combo("##CreateComponentDropdown", &selected, componentsToCreate.data(), (int)componentsToCreate.size());

				ImGui::BeginDisabled(selected == 0);
				ImGui::SameLine();
				if (ImGui::Button("Create", ImVec2(-1, 0)) && selected != 0) {
					const char* compName = componentsToCreate[selected];
					if (compName == "Mesh") obj->createComponent<Components::Mesh>();
					else if (compName == "Light") obj->createComponent<Components::Light>();
					else if (compName == "RigidBody") obj->createComponent<Components::RigidBody>();

					selected = 0;
				}
				ImGui::EndDisabled();
			} else {
				selected = 0;
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
			activeScene->getGameObjectsManager()->deleteGameObject(obj);
			explorerSelection = {};
		}
	} else {
		ImGui::Text("Root Game Object");
		if (ImGui::Button("Create GameObject", ImVec2(-1, 40))) {
			explorerSelection = { EXPLORER_SELECTION_TYPE::GAMEOBJECT, obj->createChild()->getId() };
		}
	}
}
