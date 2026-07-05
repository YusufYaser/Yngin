#include <Yngin/Core/Scenes.h>
#include <Yngin/Components/Components.h>
#include <Yngin/Rendering/Cameras.h>
#include <ImGui/imgui.h>
#include "../../Editor.h"
#include <string>
#include "../../UI/UI.h"
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/euler_angles.hpp>
#include <glm/gtx/component_wise.hpp>
#include "PropertiesWindow.h"
#include <ImGui/imgui_stdlib.h>
#include <glm/gtc/type_ptr.hpp>

using namespace Yngin;

void PropertiesWindow::showGameObjectProps(uint32_t id) {
	if (id == -1) {
		ImGui::Text("Game Objects");
		if (ImGui::Button("Create GameObject", ImVec2(-1, 40))) {
			GameObject* child = editor->activeScene->getGameObjectsManager()->getRootGameObject()->createChild();
			/*Components::Mesh* mesh = child->createComponent<Components::Mesh>();
			mesh->setModel(cubeModel);
			mesh->setTexture(gridTexture);*/
			if (child) editor->explorerSelection = { EXPLORER_SELECTION_TYPE::GAMEOBJECT, child->getId() };
		}
		return;
	}

	GameObject* obj = editor->activeScene->getGameObjectsManager()->getGameObject(id);

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
		ImGui::Text("Name");
		ImGui::SameLine(100);
		ImGui::InputText("##GameObjectName", &name, ImGuiInputTextFlags_EnterReturnsTrue);

		obj->meta.setMeta("Editor.Name", name);
	}
	ImGui::SeparatorText("Position");
	{
		glm::vec3 v = obj->getPosition();

		ImGui::SetNextItemWidth(90.0f);
		ImGui::InputFloat("##PosX", &v.x, 1.0f, 1.0f, "%.2f");
		ImGui::SameLine();
		ImGui::SetNextItemWidth(90.0f);
		ImGui::InputFloat("##PosY", &v.y, 1.0f, 1.0f, "%.2f");
		ImGui::SameLine();
		ImGui::SetNextItemWidth(90.0f);
		ImGui::InputFloat("##PosZ", &v.z, 1.0f, 1.0f, "%.2f");
		obj->setPosition(v);
	}
	if (ImGui::SmallButton("Move to Camera")) {
		obj->setPosition(editor->editorCamera->getPosition());
	}

	ImGui::SeparatorText("Rotation");
	{
		glm::vec3 v = obj->getRotation();

		const float degToRad = (3.14159265359f / 180);

		ImGui::SetNextItemWidth(90.0f);
		ImGui::InputFloat("##RotationX", &v.x, 1.0f, 22.5f, "%.1f");
		ImGui::SameLine();
		ImGui::SetNextItemWidth(90.0f);
		ImGui::InputFloat("##RotationY", &v.y, 1.0f, 22.5f, "%.1f");
		ImGui::SameLine();
		ImGui::SetNextItemWidth(90.0f);
		ImGui::InputFloat("##RotationZ", &v.z, 1.0f, 22.5f, "%.1f");
		obj->setRotation(v);
	}

	ImGui::SeparatorText("Scale");
	{
		glm::vec3 v = obj->getScale();

		ImGui::SetNextItemWidth(90.0f);
		ImGui::InputFloat("##ScaleX", &v.x, 0.1f, 1.0f, "%.2f");
		ImGui::SameLine();
		ImGui::SetNextItemWidth(90.0f);
		ImGui::InputFloat("##ScaleY", &v.y, 0.1f, 1.0f, "%.2f");
		ImGui::SameLine();
		ImGui::SetNextItemWidth(90.0f);
		ImGui::InputFloat("##ScaleZ", &v.z, 0.1f, 1.0f, "%.2f");
		obj->setScale(v);
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
			uint32_t v = mesh->getModel();

			editor->ui->modelSelector("##MeshModelID", &v);

			mesh->setModel(v);
		}

		{
			ImGui::Text("Texture");
			ImGui::SameLine(100);

			uint32_t v = mesh->getTexture();

			editor->ui->textureSelector("##MeshTextureID", &v);

			mesh->setTexture(v);
		}
		{
			glm::vec3 v = mesh->getColor();

			ImGui::Text("Color");
			ImGui::SameLine();
			ImGui::ColorButton("MeshColorPreview", ImVec4(v[0], v[1], v[2], 1), ImGuiColorEditFlags_NoDragDrop, ImVec2(50, ImGui::GetFrameHeight()));
			ImGui::SameLine();
			if (ImGui::Button("Change Color##MeshColorButton", ImVec2(-1, 0))) {
				ImGui::OpenPopup("Mesh Color Picker");
			}

			if (ImGui::BeginPopup("Mesh Color Picker")) {
				ImGui::ColorPicker3("Color##MeshColor", glm::value_ptr(v));
				ImGui::EndPopup();
			}
			mesh->setColor(v);
		}

		for (int i = 0; i < mesh->getMaterialsCount(); i++) {
			ImGui::Text("Material %i", i);
			ImGui::SameLine(100);
			{
				uint32_t v = mesh->getMaterial(i);

				editor->ui->materialSelector(("##MeshMaterial" + std::to_string(i)).c_str(), &v);
				mesh->setMaterial(i, v);
			}
		}

		if (deleted) {
			obj->deleteComponent<Components::Mesh>();
		}
	} else {
		componentsToCreate.push_back("Mesh");
	}


	{
		Components::PointLight* light = obj->getComponent<Components::PointLight>();
		if (light) {
			ImGui::SeparatorText("Point Light Component");
			ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.5f, 0, 0, 1));
			ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.75f, 0, 0, 1));
			bool deleted = ImGui::SmallButton("Delete Component##LightComp");
			ImGui::PopStyleColor(2);
			ImGui::Text("Intensity");
			ImGui::SameLine(100);
			{
				float v = light->getIntensity();

				ImGui::InputFloat("##LightIntensity", &v, 0.05f, 0.1f);
				light->setIntensity(v);
			}
			ImGui::Text("Distance");
			ImGui::SameLine(100);
			{
				float v = light->getDistance();

				ImGui::InputFloat("##LightDistance", &v, 1.0f, 2.0f);

				light->setDistance(v);
			}
			{
				glm::vec3 v = light->getColor();

				ImGui::Text("Color");
				ImGui::SameLine();
				ImGui::ColorButton("LightColorPreview", ImVec4(v[0], v[1], v[2], 1), ImGuiColorEditFlags_NoDragDrop, ImVec2(50, ImGui::GetFrameHeight()));
				ImGui::SameLine();
				if (ImGui::Button("Change Color##LightColorButton", ImVec2(-1, 0))) {
					ImGui::OpenPopup("Light Color Picker");
				}

				if (ImGui::BeginPopup("Light Color Picker")) {
					ImGui::ColorPicker3("Color##LightColor", glm::value_ptr(v));
					ImGui::EndPopup();
				}

				light->setColor(v);
			}

			if (deleted) {
				obj->deleteComponent<Components::PointLight>();
			}
		} else {
			componentsToCreate.push_back("PointLight");
		}
	}


	{
		Components::DirectionalLight* light = obj->getComponent<Components::DirectionalLight>();
		if (light) {
			ImGui::SeparatorText("Directional Light Component");
			ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.5f, 0, 0, 1));
			ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.75f, 0, 0, 1));
			bool deleted = ImGui::SmallButton("Delete Component##DirLightComp");
			ImGui::PopStyleColor(2);
			ImGui::Text("Intensity");
			ImGui::SameLine(100);
			{
				float v = light->getIntensity();

				ImGui::InputFloat("##DirLightIntensity", &v, 0.05f, 0.1f);

				light->setIntensity(v);
			}

			{
				glm::vec3 v = light->getColor();

				ImGui::Text("Color");
				ImGui::SameLine();
				ImGui::ColorButton("##DirLightColorPreview", ImVec4(v[0], v[1], v[2], 1), ImGuiColorEditFlags_NoDragDrop, ImVec2(50, ImGui::GetFrameHeight()));
				ImGui::SameLine();
				if (ImGui::Button("Change Color##DirLightColorButton", ImVec2(-1, 0))) {
					ImGui::OpenPopup("Directional Light Color Picker");
				}

				if (ImGui::BeginPopup("Directional Light Color Picker")) {
					ImGui::ColorPicker3("Color##DirLightColor", glm::value_ptr(v));
					ImGui::EndPopup();
				}

				light->setColor(v);
			}

			if (deleted) {
				obj->deleteComponent<Components::DirectionalLight>();
			}
		} else {
			componentsToCreate.push_back("DirectionalLight");
		}
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
			float v = rigidBody->getMass();

			ImGui::InputFloat("##RigidBodyMass", &v);

			rigidBody->setMass(v);
		}
		ImGui::Text("Elasticity");
		ImGui::SameLine(100);
		{
			float v = rigidBody->getElasticity();

			ImGui::InputFloat("##RigidBodyElasticity", &v);

			rigidBody->setElasticity(v);
		}
		ImGui::Text("Can Bounce?");
		ImGui::SameLine(100);
		{
			bool v = rigidBody->canBounce();

			ImGui::Checkbox("##RigidBodyCanBounce", &v);

			rigidBody->setCanBounce(v);
		}

		ImGui::Text("Velocity");
		{
			glm::vec3 v = rigidBody->getVelocity();

			ImGui::SetNextItemWidth(90.0f);
			ImGui::InputFloat("##VelocityX", &v.x, 1.0f, 1.0f, "%.2f");

			ImGui::SameLine();
			ImGui::SetNextItemWidth(90.0f);
			ImGui::InputFloat("##VelocityY", &v.y, 1.0f, 1.0f, "%.2f");

			ImGui::SameLine();
			ImGui::SetNextItemWidth(90.0f);
			ImGui::InputFloat("##VelocityZ", &v.z, 1.0f, 1.0f, "%.2f");

			rigidBody->setVelocity(v);
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
		int selected = 0;

		if (componentsToCreate.size() != 1) {
			ImGui::SeparatorText("Create Component");

			ImGui::SetNextItemWidth(-1);
			if (ImGui::Combo("##CreateComponentDropdown", &selected, componentsToCreate.data(), (int)componentsToCreate.size())) {

				const char* compName = componentsToCreate[selected];
				if (compName == "Mesh") {
					Components::Mesh* mesh = obj->createComponent<Components::Mesh>();
					if (mesh) mesh->setTexture(1);
				} else if (compName == "PointLight") obj->createComponent<Components::PointLight>();
				else if (compName == "DirectionalLight") obj->createComponent<Components::DirectionalLight>();
				else if (compName == "RigidBody") obj->createComponent<Components::RigidBody>();
				else if (compName == "BoxCollider") obj->createComponent<Components::BoxCollider>();

				selected = 0;
			}
		} else {
			selected = 0;
		}
	}

	ImGui::SeparatorText("Other");

	bool gotoClicked = ImGui::Button("Go To", ImVec2(ImGui::GetContentRegionAvail().x / 2.0f, 40));
	ImGui::SameLine();
	bool lookAtClicked = ImGui::Button("Look At", ImVec2(-1, 40));


	if (lookAtClicked || gotoClicked) {
		Services::TweenSettings settings{};
		settings.duration = 0.25f;
		settings.function = Services::TWEEN_FUNCTION::EASE_INOUT;

		glm::vec3 pos = obj->getPosition();
		glm::vec3 newOrientation = glm::normalize(pos - editor->editorCamera->getPosition());

		if (Components::Mesh* mesh = obj->getComponent<Components::Mesh>()) {
			Model* model = editor->ctx->getModelsManager()->getModel(mesh->getModel());
			if (model) {
				glm::vec3 rot = obj->getRotation();
				glm::vec3 scale = obj->getScale();

				static const glm::mat4 i(1.0f);
				glm::mat4 modelMatrix;

				modelMatrix =
					glm::yawPitchRoll(rot.y, rot.x, rot.z) *
					glm::scale(i, scale);

				pos += glm::vec3(modelMatrix * glm::vec4(model->getCenter(), 1.0f));

				newOrientation = glm::normalize(pos - editor->editorCamera->getPosition());

				float radius = model->getRadius() * glm::compMax(obj->getScale());

				pos -= newOrientation * radius * 1.5f;
			}
		}

		// TODO: replace these when I add tween rotation
		editor->ctx->getService<Services::Tween>()->tweenFloat(editor->editorCamera->getOrientation().x, newOrientation.x, settings, [this](float v) {
			glm::vec3 o = editor->editorCamera->getOrientation();
			o.x = v;
			editor->editorCamera->setOrientation(o);
			});

		editor->ctx->getService<Services::Tween>()->tweenFloat(editor->editorCamera->getOrientation().y, newOrientation.y, settings, [this](float v) {
			glm::vec3 o = editor->editorCamera->getOrientation();
			o.y = v;
			editor->editorCamera->setOrientation(o);
			});

		editor->ctx->getService<Services::Tween>()->tweenFloat(editor->editorCamera->getOrientation().z, newOrientation.z, settings, [this](float v) {
			glm::vec3 o = editor->editorCamera->getOrientation();
			o.z = v;
			editor->editorCamera->setOrientation(o);
			});

		if (gotoClicked) {
			settings.duration = 0.5f;

			editor->ctx->getService<Services::Tween>()->tweenPos(editor->editorCamera, pos, settings);
		}
	}

	ImGui::Separator();
	if (ImGui::Button("Create Child", ImVec2(-1, 40))) {
		GameObject* child = obj->createChild();
		/*Components::Mesh* mesh = child->createComponent<Components::Mesh>();
		mesh->setModel(cubeModel);
		mesh->setTexture(gridTexture);*/
		if (child) editor->explorerSelection = { EXPLORER_SELECTION_TYPE::GAMEOBJECT, child->getId() };
	}

	ImGui::Separator();
	ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.5f, 0, 0, 1));
	ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.75f, 0, 0, 1));
	if (ImGui::Button("Delete", ImVec2(-1, 40))) {
		editor->activeScene->getGameObjectsManager()->deleteGameObject(obj);
		editor->explorerSelection = {};
	}
	ImGui::PopStyleColor(2);
}
