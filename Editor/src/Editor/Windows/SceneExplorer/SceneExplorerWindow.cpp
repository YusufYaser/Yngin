#include "SceneExplorerWindow.h"

void SceneExplorerWindow::draw() {
	if (shouldClose()) return;

	if (!ImGui::Begin(("Scene Explorer###" + getWindowImGuiId()).c_str(), &isWindowOpen)) {
		ImGui::End();
		return;
	}

	// Yes, this has tabs unrelated to the scene
	// These will be removed soon and separated into different windows

	ImGui::BeginTabBar("Explorer Tabs");
	if (ImGui::BeginTabItem("Scene")) {
		showSceneExplorer();
		ImGui::EndTabItem();
	}
	if (ImGui::BeginTabItem("Game")) {
		showGameExplorer();
		ImGui::EndTabItem();
	}
	if (ImGui::BeginTabItem("Resources")) {
		showResourceExplorer();
		ImGui::EndTabItem();
	}
	ImGui::EndTabBar();

	ImGui::End();
}
