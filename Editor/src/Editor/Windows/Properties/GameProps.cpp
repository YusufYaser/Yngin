#include <ImGui/imgui.h>
#include "../../Editor.h"
#include <string>
#include "PropertiesWindow.h"

using namespace Yngin;

void PropertiesWindow::showGameProps() {
	ImGui::Text("Game Settings");
	ImGui::Separator();

	{
		static char v[32] = {};
		ImGui::Text("Game Name");
		ImGui::SameLine(100);
		if (ImGui::InputText("##GameName", v, 32)) {
			editor->gameSettings.name = v;
		} else {
			strcpy_s(v, 32, editor->gameSettings.name.c_str());
		}
	}

	{
		static bool v = false;
		ImGui::Text("Full Screen");
		ImGui::SameLine(100);
		if (ImGui::Checkbox("##GameFullscreen", &v)) {
			editor->gameSettings.fullscreen = v;
		} else {
			v = editor->gameSettings.fullscreen;
		}
	}

	if (!editor->gameSettings.fullscreen) {
		ImGui::Text("Window Size");
		ImGui::SameLine(100);

		ImGui::SetNextItemWidth(93.0f);
		{
			static int v = 0;
			if (ImGui::InputInt("##GameWindowWidth", &v, 0, 0)) {
				editor->gameSettings.windowWidth = v;
			} else {
				v = editor->gameSettings.windowWidth;
			}
		}

		ImGui::SetNextItemWidth(93.0f);
		ImGui::SameLine();
		{
			static int v = 0;
			if (ImGui::InputInt("##GameWindowHeight", &v, 0, 0)) {
				editor->gameSettings.windowHeight = v;
			} else {
				v = editor->gameSettings.windowHeight;
			}
		}
	}
}
