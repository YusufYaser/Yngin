#include <Yngin/Core/Scenes.h>
#include <Yngin/Core/Materials.h>
#include <Yngin/Components/Components.h>
#include <Yngin/Rendering/Cameras.h>
#include <ImGui/imgui.h>
#include "../Editor.h"
#include <string>
#include <fstream>
#include <sstream>
#include <glm/gtc/type_ptr.hpp>
#include <filesystem>

using namespace Yngin;

void Editor::showModelProps(uint32_t id) {
	if (explorerSelection.second == -1) {
		ImGui::Text("Models");
		ImGui::Separator();

		{
			const float degToRad = (3.14159265359f / 180);

			static glm::vec3 offset = {};
			ImGui::SeparatorText("Offset");
			ImGui::SetNextItemWidth(90.0f);
			ImGui::InputFloat("##Offset X", &offset.x, 1.0f, 1.0f, "%.1f");
			ImGui::SameLine();
			ImGui::SetNextItemWidth(90.0f);
			ImGui::InputFloat("##Offset Y", &offset.y, 1.0f, 1.0f, "%.1f");
			ImGui::SameLine();
			ImGui::SetNextItemWidth(90.0f);
			ImGui::InputFloat("##Offset Z", &offset.z, 1.0f, 1.0f, "%.1f");

			static glm::vec3 rot = {};
			ImGui::SeparatorText("Rotation");
			ImGui::SetNextItemWidth(90.0f);
			ImGui::InputFloat("##Rotation X", &rot.x, 1.0f, 22.5f, "%.1f");
			ImGui::SameLine();
			ImGui::SetNextItemWidth(90.0f);
			ImGui::InputFloat("##Rotation Y", &rot.y, 1.0f, 22.5f, "%.1f");
			ImGui::SameLine();
			ImGui::SetNextItemWidth(90.0f);
			ImGui::InputFloat("##Rotation Z", &rot.z, 1.0f, 22.5f, "%.1f");

			static glm::vec3 scale = { 1.0f, 1.0f, 1.0f };
			ImGui::SeparatorText("Scale");
			ImGui::SetNextItemWidth(90.0f);
			ImGui::InputFloat("##Scale X", &scale.x, 1.0f, 1.0f, "%.1f");
			ImGui::SameLine();
			ImGui::SetNextItemWidth(90.0f);
			ImGui::InputFloat("##Scale Y", &scale.y, 1.0f, 1.0f, "%.1f");
			ImGui::SameLine();
			ImGui::SetNextItemWidth(90.0f);
			ImGui::InputFloat("##Scale Z", &scale.z, 1.0f, 1.0f, "%.1f");

			ImGui::SeparatorText("Model File");

			{
				static char path[256] = {};

				if (ui->fileSelector("New Model", { {"Model", "*.obj"}, {"All Files", "*.*"} }, path, 256)) {
					std::ifstream modelFile(std::string(path, 256));

					if (modelFile.is_open()) {
						std::stringstream modelFileData;
						modelFileData << modelFile.rdbuf();

						modelFile.close();

						ModelData data{};
						bool loaded = ctx->getModelsManager()->parseObjFile(modelFileData.str().c_str(), modelFileData.str().length(), data);

						if (loaded) {
							glm::mat4 modelMat = glm::mat4(1.0f);
							modelMat = glm::translate(modelMat, offset);
							modelMat = glm::rotate(modelMat, rot.x * degToRad, glm::vec3(1, 0, 0));
							modelMat = glm::rotate(modelMat, rot.y * degToRad, glm::vec3(0, 1, 0));
							modelMat = glm::rotate(modelMat, rot.z * degToRad, glm::vec3(0, 0, 1));
							modelMat = glm::scale(modelMat, scale);

							for (auto& vertex : data.vertices) {
								vertex.pos = modelMat * glm::vec4(vertex.pos, 1.0f);
							}

							Model* model = ctx->getModelsManager()->createModel(data);

							std::filesystem::path fsPath(path);
							std::string filename = fsPath.filename().stem().string();
							model->meta.setMeta("Editor.Name", filename);

							explorerSelection = { EXPLORER_SELECTION_TYPE::MODEL, model->getId() };
						}

						path[0] = '\0';
						offset = {};
						rot = {};
						scale = {};
					}
				}
			}
		}

		return;
	}

	Model* model = ctx->getModelsManager()->getModel(explorerSelection.second);
	if (model == nullptr) {
		ImGui::Text("Invalid Model");
		return;
	}

	std::string name = model->meta.getMetaString("Editor.Name", std::string("Model #" + std::to_string(model->getId())));

	ImGui::Text("Properties (%s)", name.c_str());
	ImGui::Separator();
	{
		static char v[32] = {};
		ImGui::Text("Name");
		ImGui::SameLine(100);
		if (ImGui::InputText("##ObjectName", v, 32)) {
			name = v;
			model->meta.setMeta("Editor.Name", name);
		} else {
			strcpy_s(v, 32, name.c_str());
		}
	}
	ImGui::Separator();

	if (!running) {
		viewingObject = true;

		Components::Mesh* mesh = viewerObject->getComponent<Components::Mesh>();
		mesh->setModel(model);
		mesh->setTexture(2);
		viewerObject->setScale(glm::vec3(1.0f));
		viewerImage->setSize({ 0, 0, 0, 0 });
	}

	ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.5f, 0, 0, 1));
	ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.75f, 0, 0, 1));
	if (ImGui::Button("Delete", ImVec2(-1, 40))) {
		ctx->getModelsManager()->deleteModel(explorerSelection.second);
		explorerSelection = {};
	}
	ImGui::PopStyleColor(2);
}

void Editor::showMaterialProps(uint32_t id) {
	if (explorerSelection.second == -1) {
		ImGui::Text("Materials");
		ImGui::Separator();

		if (ImGui::Button("Create Material", ImVec2(-1, 40))) {
			Material* mat = ctx->getMaterialsManager()->createMaterial();
			explorerSelection = { EXPLORER_SELECTION_TYPE::MATERIAL, mat->getId() };
		}

		ImGui::SeparatorText("Load Material File");

		{
			static char v[256] = {};

			if (ui->fileSelector("New Material", { {"Material", "*.mtl"}, {"All Files", "*.*"} }, v, 256)) {
				std::string path = v;

				std::ifstream file(path);

				if (file.is_open()) {
					std::stringstream fileData;
					fileData << file.rdbuf();

					file.close();

					auto created = ctx->getMaterialsManager()->loadMtl(fileData.str().c_str(), fileData.str().length());

					std::filesystem::path fsPath(path);
					std::string filename = fsPath.filename().stem().string();

					for (auto& [name, id] : created) {
						Material* mat = ctx->getMaterialsManager()->getMaterial(id);
						if (mat == nullptr) continue;

						mat->meta.setMeta("Editor.Name", filename + ": " + name);
					}

					if (created.size() != 0) {
						explorerSelection = { EXPLORER_SELECTION_TYPE::MATERIAL, created.begin()->second };
					}

					v[0] = '\0';
				}
			}
		}
		return;
	}

	ImGui::Separator();

	Material* mat = ctx->getMaterialsManager()->getMaterial(explorerSelection.second);
	if (mat == nullptr) {
		ImGui::Text("Invalid Material");
		return;
	}

	std::string name = mat->meta.getMetaString("Editor.Name", std::string("Material #" + std::to_string(mat->getId())));

	ImGui::Text("Properties (%s)", name.c_str());
	ImGui::Separator();
	{
		static char v[32] = {};
		ImGui::Text("Name");
		ImGui::SameLine(100);
		if (ImGui::InputText("##ObjectName", v, 32)) {
			name = v;
			mat->meta.setMeta("Editor.Name", name);
		} else {
			strcpy_s(v, 32, name.c_str());
		}
	}
	ImGui::Separator();

	{
		static glm::vec3 v = {};
		ImGui::Text("Ambient Color");
		ImGui::SameLine();
		ImGui::ColorButton("Ambient Color Preview", ImVec4(v[0], v[1], v[2], 1), ImGuiColorEditFlags_NoDragDrop, ImVec2(50, ImGui::GetFrameHeight()));
		ImGui::SameLine();
		if (ImGui::Button("Change Color##AmbientColorBtn", ImVec2(-1, 0))) {
			ImGui::OpenPopup("Ambient Color Picker");
		}

		if (ImGui::BeginPopup("Ambient Color Picker")) {
			if (ImGui::ColorPicker3("Color##AmbientColor", glm::value_ptr(v))) {
				mat->setAmbientColor(v);
			}
			ImGui::EndPopup();
		} else {
			v = mat->getAmbientColor();
		}
	}

	{
		static glm::vec3 v = {};
		ImGui::Text("Diffuse Color");
		ImGui::SameLine();
		ImGui::ColorButton("Diffuse Color Preview", ImVec4(v[0], v[1], v[2], 1), ImGuiColorEditFlags_NoDragDrop, ImVec2(50, ImGui::GetFrameHeight()));
		ImGui::SameLine();
		if (ImGui::Button("Change Color##Diffuse Color Btn", ImVec2(-1, 0))) {
			ImGui::OpenPopup("Diffuse Color Picker");
		}

		if (ImGui::BeginPopup("Diffuse Color Picker")) {
			if (ImGui::ColorPicker3("Color##Diffuse Color", glm::value_ptr(v))) {
				mat->setDiffuseColor(v);
			}
			ImGui::EndPopup();
		} else {
			v = mat->getDiffuseColor();
		}
	}

	{
		static glm::vec3 v = {};
		ImGui::Text("Specular Color");
		ImGui::SameLine();
		ImGui::ColorButton("Specular Color Preview", ImVec4(v[0], v[1], v[2], 1), ImGuiColorEditFlags_NoDragDrop, ImVec2(50, ImGui::GetFrameHeight()));
		ImGui::SameLine();
		if (ImGui::Button("Change Color##Specular Color Btn", ImVec2(-1, 0))) {
			ImGui::OpenPopup("Specular Color Picker");
		}

		if (ImGui::BeginPopup("Specular Color Picker")) {
			if (ImGui::ColorPicker3("Color##Specular Color", glm::value_ptr(v))) {
				mat->setSpecularColor(v);
			}
			ImGui::EndPopup();
		} else {
			v = mat->getSpecularColor();
		}
	}

	{
		static float v = 0.0f;
		ImGui::Text("Specular Component");
		ImGui::SameLine(150);
		if (ImGui::InputFloat("##SpecularComponent", &v)) {
			mat->setSpecularComponent(v);
		} else {
			v = mat->getSpecularComponent();
		}
	}

	ImGui::Separator();

	ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.5f, 0, 0, 1));
	ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.75f, 0, 0, 1));
	ImGui::BeginDisabled(id == 0);
	if (ImGui::Button("Delete", ImVec2(-1, 0))) {
		ctx->getMaterialsManager()->deleteMaterial(id);
		explorerSelection = {};
	}
	ImGui::EndDisabled();
	ImGui::PopStyleColor(2);

	if (!running) {
		viewingObject = true;

		Components::Mesh* mesh = viewerObject->getComponent<Components::Mesh>();
		// TOOD: replace this after implementing an internal cube model
		mesh->setModel(uint32_t(0));
		mesh->setTexture(1);
		viewerObject->setScale(glm::vec3(1024.0f, 1024.0f, 1.0f));
		viewerImage->setSize({ 0, 0, 0, 0 });

		mesh->setMaterial(0, id);
	}
}

void Editor::showTextureProps(uint32_t id) {
	if (id == -1) {
		ImGui::Text("Textures");
		ImGui::Separator();
		{
			static char v[256] = {};

			if (ui->fileSelector("New Texture", { {"All Images", "*.jpg;*.jpeg;*.png;*.tga;*.dds"}, {"All Files", "*.*"} }, v, 256)) {
				std::string path = v;

				Texture* texture = ctx->getTexturesManager()->createTexture(path.c_str());

				std::filesystem::path fsPath(path);
				std::string filename = fsPath.filename().stem().string();
				texture->meta.setMeta("Editor.Name", filename);

				explorerSelection = { EXPLORER_SELECTION_TYPE::TEXTURE, texture->getId() };
				v[0] = '\0';
			}
		}

		return;
	}

	Texture* texture = ctx->getTexturesManager()->getTexture(id);
	if (texture == nullptr) {
		ImGui::Text("Invalid Texture");
		return;
	}

	std::string name = texture->meta.getMetaString("Editor.Name", std::string("Texture #" + std::to_string(texture->getId())));

	ImGui::Text("Properties (%s)", name.c_str());
	ImGui::Separator();
	{
		static char v[32] = {};
		ImGui::Text("Name");
		ImGui::SameLine(100);
		if (ImGui::InputText("##ObjectName", v, 32)) {
			name = v;
			texture->meta.setMeta("Editor.Name", name);
		} else {
			strcpy_s(v, 32, name.c_str());
		}
	}

	if (!running) {
		viewingObject = true;

		viewerObject->setScale(glm::vec3(0.0f));
		viewerImage->setSize({ 1, 0, 1, 0 });
		viewerImage->setTexture(texture);

		glm::vec2 viewportSize = ctx->getViewportSize();
		glm::ivec2 texSize = texture->getSize();
		float ratio = texSize.x * 1.0f / texSize.y;
		if (int(viewportSize.y * ratio) > viewportSize.x) {
			viewerImage->setSize({ 0, int(viewportSize.x), 0, int(viewportSize.x / ratio) });
		} else {
			viewerImage->setSize({ 0, int(viewportSize.y * ratio), 0, int(viewportSize.y) });
		}
	}

	ImGui::SeparatorText("Change Texture");
	{
		static char v[256] = {};

		if (ui->fileSelector("Change Texture##" + std::to_string(texture->getId()), { {"All Images", "*.jpg;*.jpeg;*.png;*.tga;*.dds"}, {"All Files", "*.*"} }, v, 256)) {
			std::string path = v;
			texture->setData(path.c_str());
			v[0] = '\0';
		}
	}

	ImGui::SeparatorText("Texture Settings");

	bool settingsChanged = false;

	TextureSettings settings = texture->getTextureSettings();

	{
		static bool v = false;
		ImGui::Text("Repeat");
		ImGui::SameLine(100);
		if (ImGui::Checkbox("##TextureRepeat", &v)) {
			settings.wrap = v ? TEXTURE_WRAP::REPEAT : TEXTURE_WRAP::CLAMP;
			settingsChanged = true;
		} else {
			v = settings.wrap == TEXTURE_WRAP::REPEAT;
		}
	}

	{
		static const char* componentsToCreate[] = {
			"Nearest",
			"Linear"
		};

		static int v = 0;

		ImGui::Text("Filter");
		ImGui::SameLine(100);
		if (ImGui::Combo("##TextureFilter", &v, componentsToCreate, IM_ARRAYSIZE(componentsToCreate))) {
			if (v == 0) {
				settings.filterMin = TEXTURE_FILTER::NEAREST;
				settings.filterMag = TEXTURE_FILTER::NEAREST;
			} else {
				settings.filterMin = TEXTURE_FILTER::LINEAR_MIPMAP_LINEAR;
				settings.filterMag = TEXTURE_FILTER::LINEAR;
			}
			settingsChanged = true;
		} else {
			TEXTURE_FILTER filter = settings.filterMin;
			if (filter == TEXTURE_FILTER::NEAREST) {
				v = 0;
			} else {
				v = 1;
			}
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
		ctx->getTexturesManager()->deleteTexture(id);
		explorerSelection = {};
	}
	ImGui::EndDisabled();
	ImGui::PopStyleColor(2);
}
