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

	ImGui::Text("Model");
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

					ctx->getModelsManager()->createModel(MODEL_FILE_TYPE::OBJ, modelFileData.str().c_str(), modelFileData.str().length());
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

		if (ImGui::Button("Delete", ImVec2(-1, 40))) {
			ctx->getModelsManager()->deleteModel(explorerSelection.second);
			explorerSelection = {};
		}
	}
}

void Editor::showTextureProps(uint32_t id) {
	ImGui::Text("Texture");
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
				ctx->getTexturesManager()->createTexture(path.c_str());
				v[0] = '\0';
			}
		}
	} else {
		Texture* texture = ctx->getTexturesManager()->getTexture(explorerSelection.second);
		if (texture == nullptr) return;

		viewingObject = true;
		viewerObject->getComponent<Components::Mesh>()->setModel(squareModel);

		Components::Mesh* mesh = viewerObject->getComponent<Components::Mesh>();
		mesh->setModel(squareModel);
		mesh->setTexture(texture);
		viewerObject->setScale(glm::vec3(1.0f));

		if (ImGui::Button("Delete", ImVec2(-1, 40))) {
			ctx->getTexturesManager()->deleteTexture(explorerSelection.second);
			explorerSelection = {};
		}
	}
}
