#include <ImGui/imgui.h>
#include "../Editor.h"
#include <string>

using namespace Yngin;

void Editor::showScriptProps(uint32_t id) {
	if (id == -1) {
		ImGui::Text("Game Scripts");
		if (ImGui::Button("Create Script", ImVec2(-1, 40))) {
			uint32_t newId = nextScriptId++;

			scripts[newId] = EditorScript{
				.name = "Script #" + std::to_string(newId),
				.code = ""
			};

			explorerSelection = { EXPLORER_SELECTION_TYPE::SCRIPT, newId };
		}
		return;
	}

	auto it = scripts.find(id);
	if (it == scripts.end()) {
		return;
	}

	EditorScript& script = it->second;

	ImGui::Text("Script (%s)", script.name.c_str());
	ImGui::Separator();

	{
		static char v[32] = {};
		ImGui::Text("Name");
		ImGui::SameLine(100);
		if (ImGui::InputText("##ScriptName", v, 32)) {
			script.name = v;
		} else {
			strcpy_s(v, 32, script.name.c_str());
		}
	}

	ImGui::Separator();

	ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.5f, 0, 0, 1));
	ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.75f, 0, 0, 1));
	if (ImGui::Button("Delete", ImVec2(-1, 40))) {
		scripts.erase(id);
		explorerSelection = {};
	}
	ImGui::PopStyleColor(2);
}
