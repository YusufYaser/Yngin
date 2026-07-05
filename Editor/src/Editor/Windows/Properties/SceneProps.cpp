#include "../../Editor.h"
#include <Yngin/Core/Scenes.h>
#include <ImGui/imgui.h>
#include "../../UI/UI.h"
#include "PropertiesWindow.h"
#include <ImGui/imgui_stdlib.h>
#include <glm/gtc/type_ptr.hpp>

using namespace Yngin;

void PropertiesWindow::showSceneProps(uint32_t id) {
	Scene* scene = editor->ctx->getScenesManager()->getScene(0);
	if (scene == nullptr) return;

	ImGui::Text("Properties (Scene)");
	ImGui::Separator();

	{
		ImGui::Text("Skybox Texture");
		ImGui::SameLine(150);
		auto v = scene->getSkyboxTextureId();
		ImGui::PushItemWidth(-1);
		editor->ui->textureSelector("##SkyboxTextureID", &v);

		scene->setSkyboxTexture(v);
		ImGui::PopItemWidth();
	}

	{
		LightSettings light = scene->getLightSettings();
		auto v = light.ambientLight;

		ImGui::Text("Ambient Light Color");
		ImGui::SameLine();
		ImGui::ColorButton("Ambient Light Color Preview", ImVec4(v[0], v[1], v[2], 1), ImGuiColorEditFlags_NoDragDrop, ImVec2(50, ImGui::GetFrameHeight()));
		ImGui::SameLine();
		if (ImGui::Button("Change Color##AmbientLightColorButton", ImVec2(-1, 0))) {
			ImGui::OpenPopup("Ambient Light Color Picker");
		}

		if (ImGui::BeginPopup("Ambient Light Color Picker")) {
			if (ImGui::ColorPicker3("Color##AmbientLightColor", glm::value_ptr(v))) {
				light.ambientLight = glm::vec3(v[0], v[1], v[2]);
			}
			ImGui::EndPopup();
		}

		scene->setLightSettings(light);
	}

	ImGui::Text("Gravity");
	ImGui::SameLine(100);
	{
		float v = scene->getGravity();

		ImGui::InputFloat("##SceneGravity", &v, 1.0f, 2.0f);

		scene->setGravity(v);
	}
}
