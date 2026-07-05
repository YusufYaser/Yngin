#include "PropertiesWindow.h"

void PropertiesWindow::draw() {
	if (shouldClose()) return;

	if (!ImGui::Begin(("Properties###" + getWindowImGuiId()).c_str(), &isWindowOpen)) {
		ImGui::End();
		return;
	}

	auto selection = forcedSelection.first != EXPLORER_SELECTION_TYPE::NONE ? forcedSelection : editor->explorerSelection;

	switch (selection.first) {
	case EXPLORER_SELECTION_TYPE::GAME:
		showGameProps();
		break;

	case EXPLORER_SELECTION_TYPE::SCRIPT:
		showScriptProps(selection.second);
		break;

	case EXPLORER_SELECTION_TYPE::GAMEOBJECT:
		showGameObjectProps(selection.second);
		break;

	case EXPLORER_SELECTION_TYPE::UIELEMENT:
	{
		showUIElementProps(selection.second, false);
		break;
	}

	case EXPLORER_SELECTION_TYPE::MODEL:
	{
		showModelProps(selection.second);
		break;
	}

	case EXPLORER_SELECTION_TYPE::MATERIAL:
	{
		showMaterialProps(selection.second);
		break;
	}

	case EXPLORER_SELECTION_TYPE::TEXTURE:
	{
		showTextureProps(selection.second);
		break;
	}

	case EXPLORER_SELECTION_TYPE::SCENE:
	{
		showSceneProps(selection.second);
		break;
	}
	}

	ImGui::End();
}
