#include <Yngin/Core/Scenes.h>
#include <Yngin/Core/Materials.h>
#include <Yngin/Components/Components.h>
#include <Yngin/Rendering/Cameras.h>
#include <ImGui/imgui.h>
#include "../../Editor.h"
#include <string>
#include <fstream>
#include <sstream>
#include <glm/gtc/type_ptr.hpp>
#include <filesystem>
#include "PropertiesWindow.h"
#include <ImGui/imgui_stdlib.h>

using namespace Yngin;

void PropertiesWindow::showModelProps(uint32_t id) {
	if (editor->explorerSelection.second == -1) {
		ImGui::Text("Models");
		ImGui::SeparatorText("Model Transform");

		{
			const float degToRad = (3.14159265359f / 180);

			ImGui::Text("Offset");
			ImGui::SetNextItemWidth(90.0f);
			ImGui::InputFloat("##Offset X", &newModelOffset.x, 1.0f, 1.0f, "%.1f");
			ImGui::SameLine();
			ImGui::SetNextItemWidth(90.0f);
			ImGui::InputFloat("##Offset Y", &newModelOffset.y, 1.0f, 1.0f, "%.1f");
			ImGui::SameLine();
			ImGui::SetNextItemWidth(90.0f);
			ImGui::InputFloat("##Offset Z", &newModelOffset.z, 1.0f, 1.0f, "%.1f");

			ImGui::Text("Rotation");
			ImGui::SetNextItemWidth(90.0f);
			ImGui::InputFloat("##Rotation X", &newModelRotation.x, 1.0f, 22.5f, "%.1f");
			ImGui::SameLine();
			ImGui::SetNextItemWidth(90.0f);
			ImGui::InputFloat("##Rotation Y", &newModelRotation.y, 1.0f, 22.5f, "%.1f");
			ImGui::SameLine();
			ImGui::SetNextItemWidth(90.0f);
			ImGui::InputFloat("##Rotation Z", &newModelRotation.z, 1.0f, 22.5f, "%.1f");

			ImGui::Text("Scale");
			ImGui::SetNextItemWidth(90.0f);
			ImGui::InputFloat("##Scale X", &newModelScale.x, 1.0f, 1.0f, "%.2f");
			ImGui::SameLine();
			ImGui::SetNextItemWidth(90.0f);
			ImGui::InputFloat("##Scale Y", &newModelScale.y, 1.0f, 1.0f, "%.2f");
			ImGui::SameLine();
			ImGui::SetNextItemWidth(90.0f);
			ImGui::InputFloat("##Scale Z", &newModelScale.z, 1.0f, 1.0f, "%.2f");

			ImGui::SeparatorText("Model File");

			{
				if (editor->ui->fileSelector("New Model", { {"Model", "*.obj"}, {"All Files", "*.*"} }, &path)) {
					std::ifstream modelFile(path);

					if (modelFile.is_open()) {
						std::stringstream modelFileData;
						modelFileData << modelFile.rdbuf();

						modelFile.close();

						ModelData data{};
						bool loaded = editor->ctx->getModelsManager()->parseObjFile(modelFileData.str().c_str(), modelFileData.str().length(), data);

						if (loaded) {
							glm::mat4 modelMat = glm::mat4(1.0f);
							modelMat = glm::translate(modelMat, newModelOffset);
							modelMat = glm::rotate(modelMat, newModelRotation.x * degToRad, glm::vec3(1, 0, 0));
							modelMat = glm::rotate(modelMat, newModelRotation.y * degToRad, glm::vec3(0, 1, 0));
							modelMat = glm::rotate(modelMat, newModelRotation.z * degToRad, glm::vec3(0, 0, 1));
							modelMat = glm::scale(modelMat, newModelScale);
							for (auto& vertex : data.vertices) {
								vertex.pos = modelMat * glm::vec4(vertex.pos, 1.0f);
							}

							Model* model = editor->ctx->getModelsManager()->createModel(data);

							if (model) {
								std::filesystem::path fsPath(path);
								std::string filename = fsPath.filename().stem().string();
								model->meta.setMeta("Editor.Name", filename.substr(0, 64));

								editor->explorerSelection = { EXPLORER_SELECTION_TYPE::MODEL, model->getId() };
							}
						}

						path = "";
						newModelOffset = {};
						newModelRotation = glm::vec3(90.0f, 0, 0);
						newModelScale = glm::vec3(1.0f);
					}
				}
			}
		}

		return;
	}

	Model* model = editor->ctx->getModelsManager()->getModel(editor->explorerSelection.second);
	if (model == nullptr) {
		ImGui::Text("Invalid Model");
		return;
	}

	std::string name = model->meta.getMetaString("Editor.Name", std::string("Model #" + std::to_string(model->getId())));

	ImGui::Text("Properties (%s) (%i)", name.c_str(), model->getId());
	ImGui::Separator();
	{
		ImGui::Text("Name");
		ImGui::SameLine(100);
		ImGui::InputText("##ObjectName", &name);

		model->meta.setMeta("Editor.Name", name);
	}
	ImGui::Separator();

	if (!editor->running) {
		editor->viewingObject = true;

		Components::Mesh* mesh = editor->viewerObject->getComponent<Components::Mesh>();
		mesh->setModel(model);
		mesh->setTexture(2);
		editor->viewerObject->setScale(glm::vec3(1.0f));
		editor->viewerImage->setSize({ 0, 0, 0, 0 });
	}

	bool del = false;
	ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.5f, 0, 0, 1));
	ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.75f, 0, 0, 1));
	if (ImGui::Button("Delete", ImVec2(-1, 40))) {
		del = true;
	}
	ImGui::PopStyleColor(2);

	ImGui::SeparatorText("Internal");

	ImGui::Text("Vertices: %i", model->getModelData().vertices.size());
	ImGui::Text("Submeshes: %i", model->getSubmeshesCount());

	if (del) {
		editor->ctx->getModelsManager()->deleteModel(editor->explorerSelection.second);
		editor->explorerSelection = {};
	}
}

void PropertiesWindow::showMaterialProps(uint32_t id) {
	if (editor->explorerSelection.second == -1) {
		ImGui::Text("Materials");
		ImGui::Separator();

		if (ImGui::Button("Create Material", ImVec2(-1, 40))) {
			Material* mat = editor->ctx->getMaterialsManager()->createMaterial();
			if (mat) editor->explorerSelection = { EXPLORER_SELECTION_TYPE::MATERIAL, mat->getId() };
		}

		ImGui::SeparatorText("Load Material File");

		{
			if (editor->ui->fileSelector("New Material", { {"Material", "*.mtl"}, {"All Files", "*.*"} }, &path)) {
				std::ifstream file(path);

				if (file.is_open()) {
					std::stringstream fileData;
					fileData << file.rdbuf();

					file.close();

					auto created = editor->ctx->getMaterialsManager()->loadMtl(fileData.str().c_str(), fileData.str().length());

					std::filesystem::path fsPath(path);
					std::string filename = fsPath.filename().stem().string();

					for (auto& [name, id] : created) {
						Material* mat = editor->ctx->getMaterialsManager()->getMaterial(id);
						if (mat == nullptr) continue;

						mat->meta.setMeta("Editor.Name", (filename + ": " + name).substr(0, 64));
					}

					if (created.size() != 0) {
						editor->explorerSelection = { EXPLORER_SELECTION_TYPE::MATERIAL, created.begin()->second };
					}

					path = "";
				}
			}
		}
		return;
	}

	ImGui::Separator();

	Material* mat = editor->ctx->getMaterialsManager()->getMaterial(editor->explorerSelection.second);
	if (mat == nullptr) {
		ImGui::Text("Invalid Material");
		return;
	}

	std::string name = mat->meta.getMetaString("Editor.Name", std::string("Material #" + std::to_string(mat->getId())));

	ImGui::Text("Properties (%s) (%i)", name.c_str(), mat->getId());
	ImGui::Separator();
	{
		ImGui::Text("Name");
		ImGui::SameLine(100);
		ImGui::InputText("##ObjectName", &name);

		mat->meta.setMeta("Editor.Name", name);
	}
	ImGui::Separator();

	{
		auto v = mat->getAmbientColor();

		ImGui::Text("Ambient Color");
		ImGui::SameLine();
		ImGui::ColorButton("Ambient Color Preview", ImVec4(v[0], v[1], v[2], 1), ImGuiColorEditFlags_NoDragDrop, ImVec2(50, ImGui::GetFrameHeight()));
		ImGui::SameLine();
		if (ImGui::Button("Change Color##AmbientColorBtn", ImVec2(-1, 0))) {
			ImGui::OpenPopup("Ambient Color Picker");
		}

		if (ImGui::BeginPopup("Ambient Color Picker")) {
			ImGui::ColorPicker3("Color##AmbientColor", glm::value_ptr(v));
			ImGui::EndPopup();
		}

		mat->setAmbientColor(v);
	}

	{
		auto v = mat->getDiffuseColor();

		ImGui::Text("Diffuse Color");
		ImGui::SameLine();
		ImGui::ColorButton("Diffuse Color Preview", ImVec4(v[0], v[1], v[2], 1), ImGuiColorEditFlags_NoDragDrop, ImVec2(50, ImGui::GetFrameHeight()));
		ImGui::SameLine();
		if (ImGui::Button("Change Color##Diffuse Color Btn", ImVec2(-1, 0))) {
			ImGui::OpenPopup("Diffuse Color Picker");
		}

		if (ImGui::BeginPopup("Diffuse Color Picker")) {
			ImGui::ColorPicker3("Color##Diffuse Color", glm::value_ptr(v));
			ImGui::EndPopup();
		}

		mat->setDiffuseColor(v);
	}

	{
		auto v = mat->getSpecularColor();

		ImGui::Text("Specular Color");
		ImGui::SameLine();
		ImGui::ColorButton("Specular Color Preview", ImVec4(v[0], v[1], v[2], 1), ImGuiColorEditFlags_NoDragDrop, ImVec2(50, ImGui::GetFrameHeight()));
		ImGui::SameLine();
		if (ImGui::Button("Change Color##Specular Color Btn", ImVec2(-1, 0))) {
			ImGui::OpenPopup("Specular Color Picker");
		}

		if (ImGui::BeginPopup("Specular Color Picker")) {
			ImGui::ColorPicker3("Color##Specular Color", glm::value_ptr(v));
			ImGui::EndPopup();
		}

		mat->setSpecularColor(v);
	}

	{
		auto v = mat->getSpecularComponent();

		ImGui::Text("Specular Component");
		ImGui::SameLine(150);
		ImGui::InputFloat("##SpecularComponent", &v);

		mat->setSpecularComponent(v);
	}

	ImGui::Separator();

	ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.5f, 0, 0, 1));
	ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.75f, 0, 0, 1));
	ImGui::BeginDisabled(id == 0);
	if (ImGui::Button("Delete", ImVec2(-1, 0))) {
		editor->ctx->getMaterialsManager()->deleteMaterial(id);
		editor->explorerSelection = {};
	}
	ImGui::EndDisabled();
	ImGui::PopStyleColor(2);

	if (!editor->running) {
		editor->viewingObject = true;

		Components::Mesh* mesh = editor->viewerObject->getComponent<Components::Mesh>();
		// TOOD: replace this after implementing an internal cube model
		mesh->setModel(uint32_t(0));
		mesh->setTexture(1);
		editor->viewerObject->setScale(glm::vec3(1024.0f, 1024.0f, 1.0f));
		editor->viewerImage->setSize({ 0, 0, 0, 0 });

		mesh->setMaterial(0, id);
	}
}

void PropertiesWindow::showTextureProps(uint32_t id) {
	if (id == -1) {
		ImGui::Text("Textures");
		ImGui::Separator();
		{
			if (editor->ui->fileSelector("New Texture", { {"All Images", "*.jpg;*.jpeg;*.png;*.tga;*.dds"}, {"All Files", "*.*"} }, &path)) {
				Texture* texture = editor->ctx->getTexturesManager()->createTexture(path.c_str());

				if (texture) {
					std::filesystem::path fsPath(path);
					std::string filename = fsPath.filename().stem().string();
					texture->meta.setMeta("Editor.Name", filename.substr(0, 64));

					editor->explorerSelection = { EXPLORER_SELECTION_TYPE::TEXTURE, texture->getId() };
					path = "";
				}
			}
		}

		return;
	}

	Texture* texture = editor->ctx->getTexturesManager()->getTexture(id);
	if (texture == nullptr) {
		ImGui::Text("Invalid Texture");
		return;
	}

	std::string name = texture->meta.getMetaString("Editor.Name", std::string("Texture #" + std::to_string(texture->getId())));

	ImGui::Text("Properties (%s) (%i)", name.c_str(), texture->getId());
	ImGui::Separator();
	{
		ImGui::Text("Name");
		ImGui::SameLine(100);
		ImGui::InputText("##ObjectName", &name);

		texture->meta.setMeta("Editor.Name", name);
	}

	if (!editor->running) {
		editor->viewingObject = true;

		editor->viewerObject->setScale(glm::vec3(0.0f));
		editor->viewerImage->setSize({ 1, 0, 1, 0 });
		editor->viewerImage->setTexture(texture);

		glm::vec2 viewportSize = editor->ctx->getViewportSize();
		glm::ivec2 texSize = texture->getSize();
		float ratio = texSize.x * 1.0f / texSize.y;
		if (int(viewportSize.y * ratio) > viewportSize.x) {
			editor->viewerImage->setSize({ 0, int(viewportSize.x), 0, int(viewportSize.x / ratio) });
		} else {
			editor->viewerImage->setSize({ 0, int(viewportSize.y * ratio), 0, int(viewportSize.y) });
		}
	}

	ImGui::SeparatorText("Change Texture");
	{
		if (editor->ui->fileSelector("Change Texture##" + std::to_string(texture->getId()), { {"All Images", "*.jpg;*.jpeg;*.png;*.tga;*.dds"}, {"All Files", "*.*"} }, &path)) {

			texture->setData(path.c_str());
			path = "";
		}
	}

	ImGui::SeparatorText("Texture Settings");

	bool settingsChanged = false;

	TextureSettings settings = texture->getTextureSettings();

	{
		bool v = settings.wrap == TEXTURE_WRAP::REPEAT;

		ImGui::Text("Repeat");
		ImGui::SameLine(100);
		if (ImGui::Checkbox("##TextureRepeat", &v)) {
			settingsChanged = true;
		}

		settings.wrap = v ? TEXTURE_WRAP::REPEAT : TEXTURE_WRAP::CLAMP;
	}

	{
		static const char* componentsToCreate[] = {
			"Nearest",
			"Linear"
		};

		int v = 0;

		if (settings.filterMin == TEXTURE_FILTER::NEAREST) {
			v = 0;
		} else {
			v = 1;
		}

		ImGui::Text("Filter");
		ImGui::SameLine(100);
		if (ImGui::Combo("##TextureFilter", &v, componentsToCreate, IM_ARRAYSIZE(componentsToCreate))) {
			settingsChanged = true;
		}

		if (v == 0) {
			settings.filterMin = TEXTURE_FILTER::NEAREST;
			settings.filterMag = TEXTURE_FILTER::NEAREST;
		} else {
			settings.filterMin = TEXTURE_FILTER::LINEAR_MIPMAP_LINEAR;
			settings.filterMag = TEXTURE_FILTER::LINEAR;
		}
	}

	if (settingsChanged) {
		texture->setSettings(settings);
	}

	ImGui::Separator();

	ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.5f, 0, 0, 1));
	ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.75f, 0, 0, 1));
	ImGui::BeginDisabled(id == 0 || id == 1 || id == 2);
	if (ImGui::Button("Delete", ImVec2(-1, 0))) {
		editor->ctx->getTexturesManager()->deleteTexture(id);
		editor->explorerSelection = {};
	}
	ImGui::EndDisabled();
	ImGui::PopStyleColor(2);
}
