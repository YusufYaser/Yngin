#include <ImGui/imgui.h>
#include <ImGui/imgui_internal.h>
#include <Yngin/Core/GameObject.h>
#include <Yngin/UI/UI.h>
#include <Yngin/Core/Scenes.h>
#include "../Editor.h"
#include "../Windows/Properties/PropertiesWindow.h"

using namespace Yngin;

namespace {
	GameObject* drawChildrenTree(Editor* editor, GameObject* obj) {
		GameObject* clicked = nullptr;

		std::string name = "Game Object";

		if (obj->getId() != 0) {
			name = obj->meta.getMetaString("Editor.Name", std::string("GameObject #" + std::to_string(obj->getId())));
		} else {
			name = "Game Objects";
		}

		name += "##ExplorerGameObject" + std::to_string(obj->getId());

		bool open = ImGui::TreeNodeEx(name.c_str(), ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_SpanAvailWidth | (obj->getId() == 0 ? ImGuiTreeNodeFlags_DefaultOpen : 0));

		bool deleteClicked = false;

		if (obj->getId() != 0 && ImGui::BeginPopupContextItem()) {
			if (ImGui::MenuItem("Open Properties")) {
				PropertiesWindow* window = new PropertiesWindow(editor);
				window->forcedSelection = { EXPLORER_SELECTION_TYPE::GAMEOBJECT, obj->getId() };
				editor->windows.push_back(std::unique_ptr<EditorWindow>(window));
			}

			ImGui::Separator();

			ImGui::PushStyleColor(ImGuiCol_HeaderHovered, ImVec4(0.8f, 0.1f, 0.1f, 1.0f));
			ImGui::PushStyleColor(ImGuiCol_HeaderActive, ImVec4(0.6f, 0.0f, 0.0f, 1.0f));

			if (ImGui::MenuItem("Delete")) {
				deleteClicked = true;
			}

			ImGui::PopStyleColor(2);

			ImGui::EndPopup();
		}

		if (ImGui::IsItemClicked(ImGuiMouseButton_Left) && !ImGui::IsItemToggledOpen()) {
			clicked = obj;
		}

		if (open) {
			if (!deleteClicked) {
				for (auto& child : obj->getChildren()) {
					GameObject* res = drawChildrenTree(editor, child);
					if (res) clicked = res;
				}
			}
			ImGui::TreePop();
		}

		if (obj->getId() != 0 && deleteClicked) {
			obj->getParent()->deleteChild(obj);
		}

		return clicked;
	}

	UI::UIElement* drawChildrenTree(UI::UIElement* obj) {
		UI::UIElement* clicked = nullptr;

		std::string name = "UI Element";

		if (obj->getId() != 0) {
			name = obj->meta.getMetaString("Editor.Name", std::string("UI Element #" + std::to_string(obj->getId())));
		} else {
			name = "UI Elements";
		}

		name += "##ExplorerUIElement" + std::to_string(obj->getId());

		bool open = ImGui::TreeNodeEx(name.c_str(), ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_SpanAvailWidth | (obj->getId() == 0 ? ImGuiTreeNodeFlags_DefaultOpen : 0));

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
	Scene* scene = activeScene;

	bool open = ImGui::TreeNodeEx("Scene", ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_SpanAvailWidth | ImGuiTreeNodeFlags_DefaultOpen);

	if (ImGui::IsItemClicked(ImGuiMouseButton_Left) && !ImGui::IsItemToggledOpen()) {
		explorerSelection = { EXPLORER_SELECTION_TYPE::SCENE, scene->getId() };
	}

	if (open) {
		GameObject* gameObject = drawChildrenTree(this, scene->getGameObjectsManager()->getRootGameObject());
		UI::UIElement* uiElement = drawChildrenTree(scene->getUIManager()->getRootElement());

		if (gameObject) {
			explorerSelection = { EXPLORER_SELECTION_TYPE::GAMEOBJECT, gameObject->getId() };
			if (explorerSelection.second == 0) explorerSelection.second = -1;
		}
		if (uiElement) {
			explorerSelection = { EXPLORER_SELECTION_TYPE::UIELEMENT, uiElement->getId() };
			if (explorerSelection.second == 0) explorerSelection.second = -1;
		}
		ImGui::TreePop();
	}
}
