#include <Yngin/Core/Scenes.h>
#include <Yngin/Components/Components.h>
#include <Yngin/Rendering/Cameras.h>
#include <ImGui/imgui.h>
#include "../Editor.h"
#include <string>
#include <fstream>
#include <sstream>

using namespace Yngin;

void Editor::showModelProps(uint32_t id) {
	if (id != -1) {
		ImGui::Text("Properties (Model #%i)", id);
	} else {
		ImGui::Text("Models");
	}
	ImGui::Separator();
	if (explorerSelection.second == -1) {
		// TODO: add file selector
		{
			static char v[256] = {};
			ImGui::Text("Path");
			ImGui::SameLine(50);
			ImGui::PushItemWidth(-1);
			ImGui::InputText("##New Model Path", v, 256);
			ImGui::PopItemWidth();

			if (ImGui::Button("Create Model", ImVec2(-1, 40))) {
				std::string path = v;

				std::ifstream modelFile(path);

				if (modelFile.is_open()) {
					std::stringstream modelFileData;
					modelFileData << modelFile.rdbuf();

					modelFile.close();

					Model* model = ctx->getModelsManager()->createModel(MODEL_FILE_TYPE::OBJ, modelFileData.str().c_str(), modelFileData.str().length());
					explorerSelection = { EXPLORER_SELECTION_TYPE::MODEL, model->getId() };
				}

				v[0] = '\0';
			}
		}
	} else {
		Model* model = ctx->getModelsManager()->getModel(explorerSelection.second);
		if (model == nullptr) return;

		viewingObject = true;

		Components::Mesh* mesh = viewerObject->getComponent<Components::Mesh>();
		mesh->setModel(model);
		mesh->setTexture(gridTexture);
		viewerObject->setScale(glm::vec3(1.0f));
		viewerImage->setSize({ 0, 0, 0, 0 });

		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.5f, 0, 0, 1));
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.75f, 0, 0, 1));
		if (ImGui::Button("Delete", ImVec2(-1, 40))) {
			ctx->getModelsManager()->deleteModel(explorerSelection.second);
			explorerSelection = {};
		}
		ImGui::PopStyleColor(2);
	}
}

void Editor::showTextureProps(uint32_t id) {
	if (id != -1) {
		ImGui::Text("Properties (Texture #%i)", id);
	} else {
		ImGui::Text("Textures");
	}
	ImGui::Separator();
	if (explorerSelection.second == -1) {
		// TODO: add file selector
		{
			static char v[256] = {};
			ImGui::Text("Path");
			ImGui::SameLine(50);
			ImGui::PushItemWidth(-1);
			ImGui::InputText("##New Texture Path", v, 256);
			ImGui::PopItemWidth();

			if (ImGui::Button("Create Texture", ImVec2(-1, 40))) {
				std::string path = v;
				Texture* texture = ctx->getTexturesManager()->createTexture(path.c_str());
				explorerSelection = { EXPLORER_SELECTION_TYPE::TEXTURE, texture->getId() };
				v[0] = '\0';
			}
		}
	} else {
		Texture* texture = ctx->getTexturesManager()->getTexture(explorerSelection.second);
		if (texture == nullptr) return;

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

		// TODO: add file selector
		{
			static char v[256] = {};
			ImGui::Text("Path");
			ImGui::SameLine(50);
			ImGui::PushItemWidth(-1);
			ImGui::InputText("##New Texture Path", v, 256);
			ImGui::PopItemWidth();

			if (ImGui::Button("Change Texture", ImVec2(-1, 0))) {
				std::string path = v;
				texture->setData(path.c_str());
				v[0] = '\0';
			}
		}

		ImGui::Separator();

		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.5f, 0, 0, 1));
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.75f, 0, 0, 1));
		if (ImGui::Button("Delete", ImVec2(-1, 0))) {
			ctx->getTexturesManager()->deleteTexture(explorerSelection.second);
			explorerSelection = {};
		}
		ImGui::PopStyleColor(2);
	}
}
