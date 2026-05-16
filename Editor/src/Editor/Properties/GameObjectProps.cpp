#include <Yngin/Core/Scenes.h>
#include <Yngin/Components/Components.h>
#include <Yngin/Rendering/Cameras.h>
#include <ImGui/imgui.h>
#include "../Editor.h"
#include <string>
#include "../UI/UI.h"

using namespace Yngin;

void Editor::showGameObjectProps(uint32_t id) {
	if (id == -1) {
		ImGui::Text("Game Objects");
		if (ImGui::Button("Create GameObject", ImVec2(-1, 40))) {
			GameObject* child = activeScene->getGameObjectsManager()->getRootGameObject()->createChild();
			/*Components::Mesh* mesh = child->createComponent<Components::Mesh>();
			mesh->setModel(cubeModel);
			mesh->setTexture(gridTexture);*/
			if (child) explorerSelection = { EXPLORER_SELECTION_TYPE::GAMEOBJECT, child->getId() };
		}
		return;
	}

	GameObject* obj = activeScene->getGameObjectsManager()->getGameObject(id);

	if (obj == nullptr) {
		ImGui::Text("Invalid GameObject");
		return;
	}

	std::string name = "Game Object";

	if (obj->getId() != 0) {
		name = obj->meta.getMetaString("Editor.Name", std::string("GameObject #" + std::to_string(obj->getId())));
	} else {
		name = "Game Objects";
	}

	ImGui::Text("Properties (%s) (%i)", name.c_str(), obj->getId());
	ImGui::Separator();
	{
		static char v[32] = {};
		ImGui::Text("Name");
		ImGui::SameLine(100);
		if (ImGui::InputText("##GameObjectName", v, 32)) {
			name = v;
			obj->meta.setMeta("Editor.Name", name);
		} else {
			strcpy_s(v, 32, name.c_str());
		}
	}
	ImGui::SeparatorText("Position");
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
	if (ImGui::SmallButton("Move to Camera")) {
		obj->setPosition(editorCamera->getPosition());
	}

	ImGui::SeparatorText("Rotation");
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

	ImGui::SeparatorText("Scale");
	{
		glm::vec3 scale = obj->getScale();
		static glm::vec3 v = {};

		ImGui::SetNextItemWidth(90.0f);
		if (ImGui::InputFloat("##ScaleX", &v.x, 0.1f, 1.0f, "%.2f")) {
			scale.x = v.x;
		} else {
			v.x = scale.x;
		}
		ImGui::SameLine();
		ImGui::SetNextItemWidth(90.0f);
		if (ImGui::InputFloat("##ScaleY", &v.y, 0.1f, 1.0f, "%.2f")) {
			scale.y = v.y;
		} else {
			v.y = scale.y;
		}
		ImGui::SameLine();
		ImGui::SetNextItemWidth(90.0f);
		if (ImGui::InputFloat("##ScaleZ", &v.z, 0.1f, 1.0f, "%.2f")) {
			scale.z = v.z;
		} else {
			v.z = scale.z;
		}
		obj->setScale(scale);
	}


	std::vector<const char*> componentsToCreate = { "Select Component" };

	Components::Mesh* mesh = obj->getComponent<Components::Mesh>();
	if (mesh) {
		ImGui::SeparatorText("Mesh Component");
		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.5f, 0, 0, 1));
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.75f, 0, 0, 1));
		bool deleted = ImGui::SmallButton("Delete Component##MeshComp");
		ImGui::PopStyleColor(2);
		ImGui::Text("Model");
		ImGui::SameLine(100);
		{
			static uint32_t v = 0;
			if (ui->modelSelector("##MeshModelID", &v)) {
				mesh->setModel(v);
			} else {
				v = mesh->getModel();
			}
		}

		{
			ImGui::Text("Texture");
			ImGui::SameLine(100);
			static uint32_t v = 0;
			if (ui->textureSelector("##MeshTextureID", &v)) {
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

		for (int i = 0; i < mesh->getMaterialsCount(); i++) {
			ImGui::Text("Material %i", i);
			ImGui::SameLine(100);
			{
				static uint32_t v[256] = {};
				if (ui->materialSelector(("##MeshMaterial" + std::to_string(i)).c_str(), &v[i])) {
					mesh->setMaterial(i, v[i]);
				} else {
					v[i] = mesh->getMaterial(i);
				}
			}
		}

		if (deleted) {
			obj->deleteComponent<Components::Mesh>();
		}
	} else {
		componentsToCreate.push_back("Mesh");
	}


	Components::Light* light = obj->getComponent<Components::Light>();
	if (light) {
		ImGui::SeparatorText("Light Component");
		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.5f, 0, 0, 1));
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.75f, 0, 0, 1));
		bool deleted = ImGui::SmallButton("Delete Component##LightComp");
		ImGui::PopStyleColor(2);
		ImGui::Text("Intensity");
		ImGui::SameLine(100);
		{
			static float v = 0;
			if (ImGui::InputFloat("##LightIntensity", &v, 0.05f, 0.1f)) {
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

		if (deleted) {
			obj->deleteComponent<Components::Light>();
		}
	} else {
		componentsToCreate.push_back("Light");
	}


	Components::RigidBody* rigidBody = obj->getComponent<Components::RigidBody>();
	if (rigidBody) {
		ImGui::SeparatorText("RigidBody Component");
		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.5f, 0, 0, 1));
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.75f, 0, 0, 1));
		bool deleted = ImGui::SmallButton("Delete Component##RigidBodyComp");
		ImGui::PopStyleColor(2);
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
				if (v < 0) v = 0;
				rigidBody->setElasticity(v);
			} else {
				v = rigidBody->getElasticity();
			}
		}
		ImGui::Text("Can Bounce?");
		ImGui::SameLine(100);
		{
			static bool v = 0;
			if (ImGui::Checkbox("##RigidBodyCanBounce", &v)) {
				rigidBody->setCanBounce(v);
			} else {
				v = rigidBody->canBounce();
			}
		}

		if (deleted) {
			obj->deleteComponent<Components::RigidBody>();
		}
	} else {
		componentsToCreate.push_back("RigidBody");
	}


	Components::BoxCollider* boxCollider = obj->getComponent<Components::BoxCollider>();
	if (boxCollider) {
		ImGui::SeparatorText("BoxCollider Component");
		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.5f, 0, 0, 1));
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.75f, 0, 0, 1));
		if (ImGui::SmallButton("Delete Component##BoxColliderComp")) {
			obj->deleteComponent<Components::BoxCollider>();
		}
		ImGui::PopStyleColor(2);
		ImGui::Separator();
	} else {
		componentsToCreate.push_back("BoxCollider");
	}


	{
		static int selected = 0;
		if (componentsToCreate.size() != 1) {
			ImGui::SeparatorText("Create Component");
			ImGui::Combo("##CreateComponentDropdown", &selected, componentsToCreate.data(), (int)componentsToCreate.size());

			ImGui::BeginDisabled(selected == 0);
			ImGui::SameLine();
			if (ImGui::Button("Create", ImVec2(-1, 0)) && selected != 0) {
				const char* compName = componentsToCreate[selected];
				if (compName == "Mesh") obj->createComponent<Components::Mesh>();
				else if (compName == "Light") obj->createComponent<Components::Light>();
				else if (compName == "RigidBody") obj->createComponent<Components::RigidBody>();
				else if (compName == "BoxCollider") obj->createComponent<Components::BoxCollider>();

				selected = 0;
			}
			ImGui::EndDisabled();
		} else {
			selected = 0;
		}
	}

	ImGui::SeparatorText("Other");

	if (ImGui::Button("Go To", ImVec2(ImGui::GetContentRegionAvail().x / 2.0f, 40))) {
		glm::vec3 newOrientation = glm::normalize(obj->getPosition() - editorCamera->getPosition());

		Services::TweenSettings settings{};
		settings.duration = 0.5f;
		settings.function = Services::TWEEN_FUNCTION::EASE_INOUT;

		ctx->getService<Services::Tween>()->tweenPos(editorCamera, obj->getPosition() - newOrientation * 2.0f, settings);

		settings.duration = 0.25f;

		// TODO: replace these when I add tween rotation
		ctx->getService<Services::Tween>()->tweenFloat(editorCamera->getOrientation().x, newOrientation.x, settings, [this](float v) {
			glm::vec3 o = editorCamera->getOrientation();
			o.x = v;
			editorCamera->setOrientation(o);
			});

		ctx->getService<Services::Tween>()->tweenFloat(editorCamera->getOrientation().y, newOrientation.y, settings, [this](float v) {
			glm::vec3 o = editorCamera->getOrientation();
			o.y = v;
			editorCamera->setOrientation(o);
			});

		ctx->getService<Services::Tween>()->tweenFloat(editorCamera->getOrientation().z, newOrientation.z, settings, [this](float v) {
			glm::vec3 o = editorCamera->getOrientation();
			o.z = v;
			editorCamera->setOrientation(o);
			});
	}
	ImGui::SameLine();
	if (ImGui::Button("Look At", ImVec2(-1, 40))) {
		Services::TweenSettings settings{};
		settings.duration = 0.25f;
		settings.function = Services::TWEEN_FUNCTION::EASE_INOUT;

		glm::vec3 newOrientation = glm::normalize(obj->getPosition() - editorCamera->getPosition());

		// TODO: replace these when I add tween rotation
		ctx->getService<Services::Tween>()->tweenFloat(editorCamera->getOrientation().x, newOrientation.x, settings, [this](float v) {
			glm::vec3 o = editorCamera->getOrientation();
			o.x = v;
			editorCamera->setOrientation(o);
			});

		ctx->getService<Services::Tween>()->tweenFloat(editorCamera->getOrientation().y, newOrientation.y, settings, [this](float v) {
			glm::vec3 o = editorCamera->getOrientation();
			o.y = v;
			editorCamera->setOrientation(o);
			});

		ctx->getService<Services::Tween>()->tweenFloat(editorCamera->getOrientation().z, newOrientation.z, settings, [this](float v) {
			glm::vec3 o = editorCamera->getOrientation();
			o.z = v;
			editorCamera->setOrientation(o);
			});
	}

	ImGui::Separator();
	if (ImGui::Button("Create Child", ImVec2(-1, 40))) {
		GameObject* child = obj->createChild();
		/*Components::Mesh* mesh = child->createComponent<Components::Mesh>();
		mesh->setModel(cubeModel);
		mesh->setTexture(gridTexture);*/
		if (child) explorerSelection = { EXPLORER_SELECTION_TYPE::GAMEOBJECT, child->getId() };
	}

	ImGui::Separator();
	ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.5f, 0, 0, 1));
	ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.75f, 0, 0, 1));
	if (ImGui::Button("Delete", ImVec2(-1, 40))) {
		activeScene->getGameObjectsManager()->deleteGameObject(obj);
		explorerSelection = {};
	}
	ImGui::PopStyleColor(2);
}
