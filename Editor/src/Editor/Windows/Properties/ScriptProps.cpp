#include <ImGui/imgui.h>
#include "../../Editor.h"
#include <string>
#include "PropertiesWindow.h"
#include <ImGui/imgui_stdlib.h>

using namespace Yngin;

void PropertiesWindow::showScriptProps(uint32_t id) {
	if (id == -1) {
		ImGui::Text("Game Scripts");
		if (ImGui::Button("Create Script", ImVec2(-1, 40))) {
			uint32_t newId = editor->nextScriptId++;

			editor->scripts[newId] = EditorScript{
				.name = "Script #" + std::to_string(newId),
				.code = ""
			};

			editor->explorerSelection = { EXPLORER_SELECTION_TYPE::SCRIPT, newId };
		}
		return;
	}

	auto it = editor->scripts.find(id);
	if (it == editor->scripts.end()) {
		return;
	}

	EditorScript& script = it->second;

	ImGui::Text("Script (%s)", script.name.c_str());
	ImGui::Separator();

	{
		ImGui::Text("Name");
		ImGui::SameLine(100);
		ImGui::InputText("##ScriptName", &script.name);
	}

	ImGui::Separator();

	ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.5f, 0, 0, 1));
	ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.75f, 0, 0, 1));
	if (ImGui::Button("Delete", ImVec2(-1, 40))) {
		editor->scripts.erase(id);
		editor->explorerSelection = {};
	}
	ImGui::PopStyleColor(2);
}
