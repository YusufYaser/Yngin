#include <ImGui/imgui.h>
#include <ImGui/imgui_internal.h>
#include <Yngin/Core/GameObject.h>
#include <Yngin/UI/UI.h>
#include <Yngin/Core/Scenes.h>
#include "../../Editor.h"
#include "SceneExplorerWindow.h"
#include "../Properties/PropertiesWindow.h"
#include <IconFontCppHeaders/IconsFontAwesome7.h>

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

		const char* icon = ICON_FA_CUBE;

		if (obj->hasComponent<Components::DirectionalLight>()) icon = ICON_FA_SUN;
		else if (obj->hasComponent<Components::PointLight>()) icon = ICON_FA_LIGHTBULB;
		else if (obj->hasComponent<Components::RigidBody>()) icon = ICON_FA_WEIGHT_HANGING;
		else if (obj->hasComponent<Components::BoxCollider>()) icon = ICON_FA_BORDER_ALL;
		else if (obj->hasComponent<Components::Mesh>()) icon = ICON_FA_CIRCLE_NODES;

		if (obj->getId() == 0) icon = ICON_FA_CUBES;

		ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_SpanAvailWidth | ImGuiTreeNodeFlags_FramePadding;

		if (obj->getId() == 0) flags |= ImGuiTreeNodeFlags_DefaultOpen;
		if (obj->getChildrenCount() == 0) flags |= ImGuiTreeNodeFlags_Leaf;

		bool open = ImGui::TreeNodeEx((std::string(icon) + " " + name).c_str(), flags);

		bool deleteClicked = false;

		if (obj->getId() != 0 && ImGui::BeginPopupContextItem()) {
			ImGui::MenuItem(name.c_str(), 0, false, false);
			ImGui::MenuItem(std::string("ID: " + std::to_string(obj->getId())).c_str(), 0, false, false);
			ImGui::MenuItem(std::string("Children Count: " + std::to_string(obj->getChildrenCount())).c_str(), 0, false, false);

			ImGui::Separator();

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

		const char* icon = ICON_FA_CUBE;

		if (obj->getType() == UI_TYPE::BUTTON) icon = ICON_FA_CIRCLE_DOT;
		else if (obj->getType() == UI_TYPE::IMAGE) icon = ICON_FA_IMAGE;
		else if (obj->getType() == UI_TYPE::TEXT) icon = ICON_FA_FONT;

		if (obj->getId() == 0) icon = ICON_FA_DISPLAY;

		bool open = ImGui::TreeNodeEx((std::string(icon) + " " + name).c_str(), ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_SpanAvailWidth | ImGuiTreeNodeFlags_FramePadding | (obj->getId() == 0 ? ImGuiTreeNodeFlags_DefaultOpen : 0));

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

void SceneExplorerWindow::showSceneExplorer() {
	Scene* scene = editor->activeScene;

	ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(8.0f, 4.0f));

	bool open = ImGui::TreeNodeEx(ICON_FA_EARTH_AMERICAS " Scene", ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_SpanAvailWidth | ImGuiTreeNodeFlags_FramePadding | ImGuiTreeNodeFlags_DefaultOpen);

	if (ImGui::IsItemClicked(ImGuiMouseButton_Left) && !ImGui::IsItemToggledOpen()) {
		editor->explorerSelection = { EXPLORER_SELECTION_TYPE::SCENE, scene->getId() };
	}

	if (open) {
		GameObject* gameObject = drawChildrenTree(editor, scene->getGameObjectsManager()->getRootGameObject());
		UI::UIElement* uiElement = drawChildrenTree(scene->getUIManager()->getRootElement());

		if (gameObject) {
			editor->explorerSelection = { EXPLORER_SELECTION_TYPE::GAMEOBJECT, gameObject->getId() };
			if (editor->explorerSelection.second == 0) editor->explorerSelection.second = -1;
		}
		if (uiElement) {
			editor->explorerSelection = { EXPLORER_SELECTION_TYPE::UIELEMENT, uiElement->getId() };
			if (editor->explorerSelection.second == 0) editor->explorerSelection.second = -1;
		}
		ImGui::TreePop();
	}

	ImGui::PopStyleVar();
}
