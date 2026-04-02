#include <ImGui/imgui.h>
#include <ImGui/imgui_internal.h>
#include "../Editor.h"

using namespace Yngin;

void Editor::showGameExplorer() {
	bool open = ImGui::TreeNodeEx("Game", ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_SpanAvailWidth | ImGuiTreeNodeFlags_DefaultOpen);

	if (ImGui::IsItemClicked(ImGuiMouseButton_Left) && !ImGui::IsItemToggledOpen()) {
		explorerSelection = { EXPLORER_SELECTION_TYPE::GAME, 0 };
	}

	if (open) {
		ImGui::TreePop();
	}
}
