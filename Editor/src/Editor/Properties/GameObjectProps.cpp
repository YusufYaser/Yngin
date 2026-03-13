#include <Yngin/Core/Scenes.h>
#include <Yngin/Components/Components.h>
#include <Yngin/Rendering/Cameras.h>
#include <ImGui/imgui.h>
#include "../Editor.h"

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

		ImGui::Text("Mesh Component");
		Components::Mesh* mesh = obj->getComponent<Components::Mesh>();
		if (mesh) {
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
					if (ImGui::ColorPicker3("Color##MeshColor", v)) {
						mesh->setColor(glm::vec3(v[0], v[1], v[2]));
					} else {
						v[0] = mesh->getColor()[0];
						v[1] = mesh->getColor()[1];
						v[2] = mesh->getColor()[2];
					}
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
				{
					static float v = 0;
					if (ImGui::InputFloat("##LightIntensity", &v)) {
						light->setIntensity(v);
					} else {
						v = light->getIntensity();
					}
				}
				ImGui::Text("Distance");
				ImGui::SameLine();
				{
					static float v = 0;
					if (ImGui::InputFloat("##LightDistance", &v)) {
						light->setDistance(v);
					} else {
						v = light->getDistance();
					}
				}
				{
					static float v[3] = {};
					if (ImGui::ColorPicker3("Color##LightColor", v)) {
						light->setColor(glm::vec3(v[0], v[1], v[2]));
					} else {
						v[0] = light->getColor()[0];
						v[1] = light->getColor()[1];
						v[2] = light->getColor()[2];
					}
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
				{
					static float v = 0;
					if (ImGui::InputFloat("##RigidBodyMass", &v)) {
						rigidBody->setMass(v);
					} else {
						v = rigidBody->getMass();
					}
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
