#include "ScriptOutputWindow.h"
#include <Yngin/Core/Scripting.h>
#include <format>
#include <ImGui/imgui_stdlib.h>

void ScriptOutputWindow::draw() {
	if (shouldClose()) return;

	ImGui::Begin(getGuiId("Output#").c_str(), &isWindowOpen);

	int frameHeight = ImGui::GetFrameHeight();

	std::string logs = "";
	int i = 0;

	for (auto& [id, log] : editor->ctx->getScriptsManager()->getGlobalOutput()) {
		if (i++ < logsStart) {
			continue;
		}
		std::string source = id == -1 ? "ScriptsManager" : std::format("Script #{}", id);

		std::string line = "[" + source + "] " + log + "\n";

		if (line.find(std::string(filter)) != std::string::npos) {
			logs.insert(0, line);
		}
	}

	char* temp = new char[logs.size() + 1];
	memcpy(temp, logs.c_str(), logs.size());
	temp[logs.size()] = '\0';

	ImGui::Text("Filter");
	ImGui::SameLine();
	ImGui::SetNextItemWidth(ImGui::GetWindowWidth() - 113.0f);
	ImGui::InputText("##Filter", &filter);

	ImGui::SameLine(ImGui::GetWindowWidth() - 50.0f);

	if (ImGui::Button("Clear")) {
		logsStart = i;
	}

	ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0, 0, 0, 0));
	ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 2.0f);
	ImGui::InputTextMultiline("##Output Text", temp, logs.size() + 1, ImVec2(-1, -frameHeight - 4), ImGuiInputTextFlags_ReadOnly);
	ImGui::PopStyleColor();
	ImGui::PopStyleVar();

	delete[] temp;

	{
		ImGui::PushItemWidth(-1);
		if (ImGui::InputText("##Global Execute", &execute, ImGuiInputTextFlags_EnterReturnsTrue)) {
			ImGui::SetKeyboardFocusHere(-1);
			editor->ctx->getScriptsManager()->execute(execute.c_str());
			execute = "";
		}
		ImGui::PopItemWidth();
	}

	ImGui::End();
}
