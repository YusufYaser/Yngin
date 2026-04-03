#include <ImGui/imgui.h>
#include <ImGui/imgui_internal.h>
#include <Yngin/Core/Models.h>
#include <Yngin/Core/Scripting.h>
#include <Yngin/Rendering/Textures.h>
#include "../Editor.h"

using namespace Yngin;

namespace {
	uint32_t drawModelsTree(std::vector<Model*> models) {
		uint32_t clicked = -2;
		bool open = ImGui::TreeNodeEx("Models", ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_SpanAvailWidth | ImGuiTreeNodeFlags_DefaultOpen);

		if (ImGui::IsItemClicked(ImGuiMouseButton_Left) && !ImGui::IsItemToggledOpen()) {
			clicked = -1;
		}

		if (open) {
			for (auto& model : models) {
				ImGui::TreeNodeEx(std::string("Model #" + std::to_string(model->getId())).c_str(), ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen);

				if (ImGui::IsItemClicked(ImGuiMouseButton_Left) && !ImGui::IsItemToggledOpen()) {
					clicked = model->getId();
				}
			}
			ImGui::TreePop();

		}

		return clicked;
	}

	uint32_t drawTexturesTree(std::vector<Texture*> textures) {
		uint32_t clicked = -2;
		bool open = ImGui::TreeNodeEx("Textures", ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_SpanAvailWidth | ImGuiTreeNodeFlags_DefaultOpen);

		if (ImGui::IsItemClicked(ImGuiMouseButton_Left) && !ImGui::IsItemToggledOpen()) {
			clicked = -1;
		}

		if (open) {
			for (auto& texture : textures) {
				ImGui::TreeNodeEx(std::string("Texture #" + std::to_string(texture->getId())).c_str(), ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen);

				if (ImGui::IsItemClicked(ImGuiMouseButton_Left) && !ImGui::IsItemToggledOpen()) {
					clicked = texture->getId();
				}
			}
			ImGui::TreePop();
		}

		return clicked;
	}

	uint32_t drawScriptsTree(std::map<uint32_t, EditorScript> scripts) {
		uint32_t clicked = -2;
		bool open = ImGui::TreeNodeEx("Scripts", ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_SpanAvailWidth | ImGuiTreeNodeFlags_DefaultOpen);

		if (ImGui::IsItemClicked(ImGuiMouseButton_Left) && !ImGui::IsItemToggledOpen()) {
			clicked = -1;
		}

		if (open) {
			for (auto& [id, script] : scripts) {
				ImGui::TreeNodeEx(script.name.c_str(), ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen);

				if (ImGui::IsItemClicked(ImGuiMouseButton_Left) && !ImGui::IsItemToggledOpen()) {
					clicked = id;
				}
			}
			ImGui::TreePop();
		}

		return clicked;
	}
}

void Editor::showResourceExplorer() {
	uint32_t model = drawModelsTree(ctx->getModelsManager()->getModels());
	if (model != -2) {
		explorerSelection = { EXPLORER_SELECTION_TYPE::MODEL, model };
	}
	uint32_t texture = drawTexturesTree(ctx->getTexturesManager()->getTextures());
	if (texture != -2) {
		explorerSelection = { EXPLORER_SELECTION_TYPE::TEXTURE, texture };
	}
	uint32_t script = drawScriptsTree(scripts);
	if (script != -2) {
		explorerSelection = { EXPLORER_SELECTION_TYPE::SCRIPT, script };
	}
}
