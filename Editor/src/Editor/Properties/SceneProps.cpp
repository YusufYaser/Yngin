#include "../Editor.h"
#include <Yngin/Core/Scenes.h>
#include <ImGui/imgui.h>

using namespace Yngin;

void Editor::showSceneProps(uint32_t id) {
	Scene* scene = ctx->getScenesManager()->getScene(0);
	if (scene == nullptr) return;

	ImGui::Text("Properties (Scene)");
	ImGui::Separator();

	ImGui::Text("Gravity");
	ImGui::SameLine(100);
	{
		static float v = 0;
		if (ImGui::InputFloat("##SceneGravity", &v, 1.0f, 2.0f)) {
			scene->setGravity(v);
		} else {
			v = scene->getGravity();
		}
	}

	{
		static float v[3] = {};
		ImGui::Text("Ambient Light Color");
		ImGui::SameLine();
		ImGui::ColorButton("Ambient Light Color Preview", ImVec4(v[0], v[1], v[2], 1), ImGuiColorEditFlags_NoDragDrop, ImVec2(50, ImGui::GetFrameHeight()));
		ImGui::SameLine();
		if (ImGui::Button("Change Color##AmbientLightColorButton", ImVec2(-1, 0))) {
			ImGui::OpenPopup("Ambient Light Color Picker");
		}

		LightSettings light = scene->getLightSettings();

		if (ImGui::BeginPopup("Ambient Light Color Picker")) {
			if (ImGui::ColorPicker3("Color##AmbientLightColor", v)) {
				light.ambientLight = glm::vec3(v[0], v[1], v[2]);
			}
			ImGui::EndPopup();
		} else {
			v[0] = light.ambientLight[0];
			v[1] = light.ambientLight[1];
			v[2] = light.ambientLight[2];
		}

		scene->setLightSettings(light);
	}
}
