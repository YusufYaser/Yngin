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

		std::string name = obj->getId() != 0 ? std::string("GameObject #" + std::to_string(obj->getId())) : "Game Objects";

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

		std::string name = obj->getId() != 0 ? std::string("UI Element #" + std::to_string(obj->getId())) : "UI Elements";

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

void Editor::showExplorer() {
	ImGuiViewport* viewport = ImGui::GetMainViewport();

	if (ImGui::BeginViewportSideBar("##Explorer", viewport, ImGuiDir_Left, 250.0f, 0)) {
		ImGui::Text("Scene Explorer");
		ImGui::Separator();
		if (ImGui::TreeNodeEx("Scene", ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_SpanAvailWidth)) {
			GameObject* gameObject = drawChildrenTree(activeScene->getGameObjectsManager()->getRootGameObject());
			UI::UIElement* uiElement = drawChildrenTree(activeScene->getUIManager()->getRootElement());
			ImGui::TreePop();
			if (gameObject) explorerSelection = { EXPLORER_SELECTION_TYPE::GAMEOBJECT, gameObject->getId() };
			if (uiElement) explorerSelection = { EXPLORER_SELECTION_TYPE::UIELEMENT, uiElement->getId() };
		}
		ImGui::End();
	}
}
