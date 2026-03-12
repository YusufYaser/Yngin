#include "Explorer.h"
#include <ImGui/imgui.h>
#include <ImGui/imgui_internal.h>
#include <Yngin/Core/GameObject.h>
#include <Yngin/UI/UI.h>

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

std::pair<EXPLORER_SELECTION_TYPE, int> showExplorer(Yngin::Scene* scene) {
	ImGuiViewport* viewport = ImGui::GetMainViewport();

	std::pair<EXPLORER_SELECTION_TYPE, int> selection;

	if (ImGui::BeginViewportSideBar("##Explorer", viewport, ImGuiDir_Left, 250.0f, 0)) {
		ImGui::Text("Scene Explorer");
		ImGui::Separator();
		if (ImGui::TreeNodeEx("Scene", ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_SpanAvailWidth)) {
			GameObject* gameObject = drawChildrenTree(scene->getGameObjectsManager()->getRootGameObject());
			UI::UIElement* uiElement = drawChildrenTree(scene->getUIManager()->getRootElement());
			ImGui::TreePop();
			if (gameObject) selection = { EXPLORER_SELECTION_TYPE::GAMEOBJECT, gameObject->getId() };
			if (uiElement) selection = { EXPLORER_SELECTION_TYPE::UIELEMENT, uiElement->getId() };
		}
		ImGui::End();
	}

	return selection;
}
