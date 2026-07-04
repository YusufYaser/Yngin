#include "ContextInfoWindow.h"
#include <Yngin/Yngin.h>

void ContextInfoWindow::draw() {
	if (shouldClose()) return;

	ImGui::Begin(getGuiId("Context Info#").c_str(), &isWindowOpen);

	if (ImGui::BeginTable("Context Info", 3, ImGuiTableFlags_Borders, ImVec2(ImGui::GetContentRegionAvail().x * 0.5f, 0))) {
		ImGui::TableSetupColumn("Name");
		ImGui::TableSetupColumn("Value");
		ImGui::TableSetupColumn("Max");

		ImGui::TableHeadersRow();

		std::vector<std::tuple<std::string, size_t, size_t>> rows;

		rows.push_back({ "Models", editor->ctx->getModelsManager()->getModelsCount(), editor->ctx->getModelsManager()->getMaxModelsCount() });
		rows.push_back({ "Materials", editor->ctx->getMaterialsManager()->getMaterialsCount(), editor->ctx->getMaterialsManager()->getMaxMaterialsCount() });
		rows.push_back({ "Textures", editor->ctx->getTexturesManager()->getTexturesCount(), editor->ctx->getTexturesManager()->getMaxTexturesCount() });
		rows.push_back({ "Loaded Scripts", editor->ctx->getScriptsManager()->getScriptsCount(), 0 });

		rows.push_back({ "Global UI Elements", editor->ctx->getGlobalUIManager()->getElementsCount(), 0 });

		for (auto& row : rows) {
			ImGui::TableNextRow();

			ImGui::TableSetColumnIndex(0);
			ImGui::Text("%s", std::get<0>(row).c_str());
			ImGui::TableSetColumnIndex(1);
			ImGui::Text("%i", std::get<1>(row));
			ImGui::TableSetColumnIndex(2);
			if (std::get<2>(row) != 0) ImGui::Text("%i", std::get<2>(row));
		}

		ImGui::EndTable();
	}

	ImGui::SameLine();

	if (ImGui::BeginTable("Scene Info", 3, ImGuiTableFlags_Borders, ImVec2(ImGui::GetContentRegionAvail().x, 0))) {
		ImGui::TableSetupColumn("Name");
		ImGui::TableSetupColumn("Value");
		ImGui::TableSetupColumn("Max");

		ImGui::TableHeadersRow();

		std::vector<std::tuple<std::string, size_t, size_t>> rows;

		rows.push_back({ "Game Objects", editor->activeScene->getGameObjectsManager()->getGameObjectsCount(), 0 });
		rows.push_back({ "Scene UI Elements", editor->activeScene->getUIManager()->getElementsCount(), 0 });
		rows.push_back({ "Submeshes Rendered", editor->ctx->getRenderer()->getSubmeshesRendered(), 0 });
		rows.push_back({ "Lights", editor->ctx->getRenderer()->getSceneLightsCount(), 0 });

		for (auto& row : rows) {
			ImGui::TableNextRow();

			ImGui::TableSetColumnIndex(0);
			ImGui::Text("%s", std::get<0>(row).c_str());
			ImGui::TableSetColumnIndex(1);
			ImGui::Text("%i", std::get<1>(row));
			ImGui::TableSetColumnIndex(2);
			if (std::get<2>(row) != 0) ImGui::Text("%i", std::get<2>(row));
		}

		ImGui::EndTable();
	}

	ImGui::End();
}
