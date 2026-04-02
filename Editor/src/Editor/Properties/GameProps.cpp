#include <ImGui/imgui.h>
#include "../Editor.h"
#include <string>

using namespace Yngin;

void Editor::showGameProps() {
	ImGui::Text("Game Settings");
	ImGui::Separator();

	{
		static char v[32] = {};
		ImGui::Text("Game Name");
		ImGui::SameLine(100);
		if (ImGui::InputText("##GameName", v, 32)) {
			gameSettings.name = v;
		} else {
			strcpy_s(v, 32, gameSettings.name.c_str());
		}
	}

	{
		static bool v = false;
		ImGui::Text("Full Screen");
		ImGui::SameLine(100);
		if (ImGui::Checkbox("##GameFullscreen", &v)) {
			gameSettings.fullscreen = v;
		} else {
			v = gameSettings.fullscreen;
		}
	}

	if (!gameSettings.fullscreen) {
		ImGui::Text("Window Size");
		ImGui::SameLine(100);

		ImGui::SetNextItemWidth(93.0f);
		{
			static int v = 0;
			if (ImGui::InputInt("##GameWindowWidth", &v, 0, 0)) {
				gameSettings.windowWidth = v;
			} else {
				v = gameSettings.windowWidth;
			}
		}

		ImGui::SetNextItemWidth(93.0f);
		ImGui::SameLine();
		{
			static int v = 0;
			if (ImGui::InputInt("##GameWindowHeight", &v, 0, 0)) {
				gameSettings.windowHeight = v;
			} else {
				v = gameSettings.windowHeight;
			}
		}
	}
}
