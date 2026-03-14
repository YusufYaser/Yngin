#include <ImGui/imgui.h>
#include <ImGui/imgui_internal.h>
#include <Yngin/Core/GameObject.h>
#include <Yngin/UI/UI.h>
#include <Yngin/Core/Scenes.h>
#include "Editor.h"

using namespace Yngin;

namespace {
	GameObject* drawChildrenTree(GameObject* obj) {
		GameObject* clicked = nullptr;

		std::string name = "Game Object";

		if (obj->getId() != 0) {
			name = obj->getMetaString("Editor.Name", std::string("GameObject #" + std::to_string(obj->getId())));
		} else {
			name = "Game Objects";
		}

		name += "##ExplorerGameObject" + std::to_string(obj->getId());

		bool open = ImGui::TreeNodeEx(name.c_str(), ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_SpanAvailWidth);

		if (ImGui::IsItemClicked(ImGuiMouseButton_Left) && !ImGui::IsItemToggledOpen()) {
			clicked = obj;
		}

		if (open) {
			for (auto& child : obj->getChildren()) {
				GameObject* res = drawChildrenTree(child);
				if (res) clicked = res;
			}
			ImGui::TreePop();
		}

		return clicked;
	}

	UI::UIElement* drawChildrenTree(UI::UIElement* obj) {
		UI::UIElement* clicked = nullptr;

		std::string name = "UI Element";

		if (obj->getId() != 0) {
			name = obj->getMetaString("Editor.Name", std::string("UI Element #" + std::to_string(obj->getId())));
		} else {
			name = "UI Elements";
		}

		name += "##ExplorerUIElement" + std::to_string(obj->getId());

		bool open = ImGui::TreeNodeEx(name.c_str(), ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_SpanAvailWidth);

		if (ImGui::IsItemClicked(ImGuiMouseButton_Left) && !ImGui::IsItemToggledOpen()) {
			clicked = obj;
		}

		if (open) {
			for (auto& child : obj->getChildren()) {
				UI::UIElement* res = drawChildrenTree(child);
				if (res) clicked = res;
			}
			ImGui::TreePop();
		}

		return clicked;
	}
}

void Editor::showSceneExplorer() {
	GameObject* gameObject = drawChildrenTree(activeScene->getGameObjectsManager()->getRootGameObject());
	UI::UIElement* uiElement = drawChildrenTree(activeScene->getUIManager()->getRootElement());

	if (gameObject) {
		explorerSelection = { EXPLORER_SELECTION_TYPE::GAMEOBJECT, gameObject->getId() };
		if (explorerSelection.second == 0) explorerSelection.second = -1;
	}
	if (uiElement) {
		explorerSelection = { EXPLORER_SELECTION_TYPE::UIELEMENT, uiElement->getId() };
		if (explorerSelection.second == 0) explorerSelection.second = -1;
	}
}
