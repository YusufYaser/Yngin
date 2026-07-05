#include "PropertiesWindow.h"

void PropertiesWindow::draw() {
	if (shouldClose()) return;

	if (!ImGui::Begin(("Properties###" + getWindowImGuiId()).c_str(), &isWindowOpen)) {
		ImGui::End();
		return;
	}

	switch (editor->explorerSelection.first) {
	case EXPLORER_SELECTION_TYPE::GAME:
		showGameProps();
		break;

	case EXPLORER_SELECTION_TYPE::SCRIPT:
		showScriptProps(editor->explorerSelection.second);
		break;

	case EXPLORER_SELECTION_TYPE::GAMEOBJECT:
		showGameObjectProps(editor->explorerSelection.second);
		break;

	case EXPLORER_SELECTION_TYPE::UIELEMENT:
	{
		showUIElementProps(editor->explorerSelection.second, false);
		break;
	}

	case EXPLORER_SELECTION_TYPE::MODEL:
	{
		showModelProps(editor->explorerSelection.second);
		break;
	}

	case EXPLORER_SELECTION_TYPE::MATERIAL:
	{
		showMaterialProps(editor->explorerSelection.second);
		break;
	}

	case EXPLORER_SELECTION_TYPE::TEXTURE:
	{
		showTextureProps(editor->explorerSelection.second);
		break;
	}

	case EXPLORER_SELECTION_TYPE::SCENE:
	{
		showSceneProps(editor->explorerSelection.second);
		break;
	}
	}

	ImGui::End();
}
