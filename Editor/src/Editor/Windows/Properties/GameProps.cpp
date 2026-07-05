#include <ImGui/imgui.h>
#include "../../Editor.h"
#include <string>
#include "PropertiesWindow.h"
#include <ImGui/imgui_stdlib.h>

using namespace Yngin;

void PropertiesWindow::showGameProps() {
	ImGui::Text("Game Settings");
	ImGui::Separator();

	{
		ImGui::Text("Game Name");
		ImGui::SameLine(100);
		ImGui::InputText("##GameName", &editor->gameSettings.name);
	}

	{
		ImGui::Text("Full Screen");
		ImGui::SameLine(100);
		ImGui::Checkbox("##GameFullscreen", &editor->gameSettings.fullscreen);
	}

	if (!editor->gameSettings.fullscreen) {
		ImGui::Text("Window Size");
		ImGui::SameLine(100);

		ImGui::SetNextItemWidth(93.0f);
		{
			ImGui::InputInt("##GameWindowWidth", &editor->gameSettings.windowWidth, 0, 0);
		}

		ImGui::SetNextItemWidth(93.0f);
		ImGui::SameLine();
		{
			ImGui::InputInt("##GameWindowHeight", &editor->gameSettings.windowHeight, 0, 0);
		}
	}
}
