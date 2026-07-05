#include <Yngin/UI/UI.h>
#include <ImGui/imgui.h>
#include "../../Editor.h"
#include <string>
#include "../../UI/UI.h"
#include "PropertiesWindow.h"
#include <ImGui/imgui_stdlib.h>
#include <glm/gtc/type_ptr.hpp>

using namespace Yngin;

void PropertiesWindow::showUIElementProps(uint32_t id, bool global) {
	UI::UIManager* mgr = editor->activeScene->getUIManager();
	if (global) mgr = editor->ctx->getGlobalUIManager();

	static const std::vector<const char*> elementTypes = {
		"Select Element Type",
		"Element",
		"Image",
		"Text",
		"Button"
	};

	if (id == -1) {
		ImGui::Text("Scene UI Elements");

		int selectedElementType = 0;

		ImGui::SetNextItemWidth(-1);
		if (ImGui::Combo("##UIElementType", &selectedElementType, elementTypes.data(), (int)elementTypes.size())) {
			UI::UIElement* child = nullptr;

			switch (selectedElementType) {
			case 1:
				child = mgr->getRootElement()->createChild();
				break;
			case 2:
				child = mgr->getRootElement()->createChild<UI::Image>();
				break;
			case 3:
				child = mgr->getRootElement()->createChild<UI::Text>();
				break;
			case 4:
				child = mgr->getRootElement()->createChild<UI::Button>();
				child->setPivot({ 1, 1 });
				break;
			}
			child->meta.setMeta("Editor.Name", std::string(elementTypes[selectedElementType]).append(" #" + std::to_string(child->getId())));

			if (child) editor->explorerSelection = { EXPLORER_SELECTION_TYPE::UIELEMENT, child->getId() };
		}

		return;
	}

	UI::UIElement* element = mgr->getElement(id);
	if (element == nullptr) {
		ImGui::Text("Invalid UI Element");
		return;
	}

	std::string name = "UI Element";

	name = element->meta.getMetaString("Editor.Name", std::string("UI Element #" + std::to_string(element->getId())));

	ImGui::Text("Properties (%s) (%i)", name.c_str(), element->getId());
	ImGui::Separator();
	{
		ImGui::Text("Name");
		ImGui::SameLine(100);
		ImGui::InputText("##UIName", &name);

		element->meta.setMeta("Editor.Name", name);
	}

	ImGui::SeparatorText("Position");
	{
		UI::UITransform v = element->getPosition();

		ImGui::Text("Scale");
		ImGui::SameLine(100);

		ImGui::SetNextItemWidth(90.0f);
		ImGui::InputFloat("##PosXScale", &v.xScale, 0.05f, 0.05f, "%.2f");

		ImGui::SameLine();
		ImGui::SetNextItemWidth(90.0f);
		ImGui::InputFloat("##PosYScale", &v.yScale, 0.05f, 0.05f, "%.2f");


		ImGui::Text("Offset");
		ImGui::SameLine(100);

		ImGui::SetNextItemWidth(90.0f);
		ImGui::InputInt("##PosXOffset", &v.xOffset);

		ImGui::SameLine();
		ImGui::SetNextItemWidth(90.0f);
		ImGui::InputInt("##PosYOffset", &v.yOffset);

		element->setPosition(v);
	}

	ImGui::SeparatorText("Pivot");
	{
		glm::vec2 v = element->getPivot();

		ImGui::SetNextItemWidth(136.0f);
		ImGui::InputFloat("##PivotX", &v.x, 0.05f, 0.05f, "%.2f");

		ImGui::SameLine();
		ImGui::SetNextItemWidth(136.0f);
		ImGui::InputFloat("##PivotY", &v.y, 0.05f, 0.05f, "%.2f");

		element->setPivot(v);
	}

	ImGui::SeparatorText("Size");
	{
		UI::UITransform v = element->getSize();

		ImGui::Text("Scale");
		ImGui::SameLine(100);

		ImGui::SetNextItemWidth(90.0f);
		ImGui::InputFloat("##SizeXScale", &v.xScale, 0.05f, 0.05f, "%.2f");

		ImGui::SameLine();
		ImGui::SetNextItemWidth(90.0f);
		ImGui::InputFloat("##SizeYScale", &v.yScale, 0.05f, 0.05f, "%.2f");


		ImGui::Text("Offset");
		ImGui::SameLine(100);

		ImGui::SetNextItemWidth(90.0f);
		ImGui::InputInt("##SizeXOffset", &v.xOffset);

		ImGui::SameLine();
		ImGui::SetNextItemWidth(90.0f);
		ImGui::InputInt("##SizeYOffset", &v.yOffset);

		element->setSize(v);
	}

	ImGui::SeparatorText("Color");
	{
		auto v = element->getColor();

		ImGui::Text("Color");
		ImGui::SameLine();
		ImGui::ColorButton("UI Element Color Preview", ImVec4(v[0], v[1], v[2], 1), ImGuiColorEditFlags_NoDragDrop, ImVec2(50, ImGui::GetFrameHeight()));
		ImGui::SameLine();
		if (ImGui::Button("Change Color##UIColorButton", ImVec2(-1, 0))) {
			ImGui::OpenPopup("UI Element Color Picker");
		}

		if (ImGui::BeginPopup("UI Element Color Picker")) {
			ImGui::ColorPicker3("Color##UIColor", glm::value_ptr(v));
			ImGui::EndPopup();
		}

		element->setColor(glm::vec4(v.x, v.y, v.z, 1.0f));
	}


	if (element->getType() != UI_TYPE::TEXT) {
		ImGui::SeparatorText("Crop");
		{
			UI::UICrop v = element->getCrop();

			ImGui::Text("Start");
			ImGui::SameLine(100);

			ImGui::SetNextItemWidth(90.0f);
			ImGui::InputFloat("##CropStartX", &v.start.x, 0.05f, 0.05f);

			ImGui::SameLine();
			ImGui::SetNextItemWidth(90.0f);
			ImGui::InputFloat("##CropStartY", &v.start.y, 0.05f, 0.05f);


			ImGui::Text("End");
			ImGui::SameLine(100);

			ImGui::SetNextItemWidth(90.0f);
			ImGui::InputFloat("##CropEndX", &v.end.x, 0.05f, 0.05f);

			ImGui::SameLine();
			ImGui::SetNextItemWidth(90.0f);
			ImGui::InputFloat("##CropEndY", &v.end.y, 0.05f, 0.05f);

			element->setCrop(v);
		}
	}


	bool button = element->getType() == UI_TYPE::BUTTON;
	switch (element->getType()) {
	case UI_TYPE::BUTTON:
	{
		UI::Button* button = dynamic_cast<UI::Button*>(element);

		ImGui::SeparatorText("Button");
		{
			auto v = button->getHoverColor();

			ImGui::Text("Hover Color");
			ImGui::SameLine();
			ImGui::ColorButton("Hover Color Preview", ImVec4(v[0], v[1], v[2], 1), ImGuiColorEditFlags_NoDragDrop, ImVec2(50, ImGui::GetFrameHeight()));
			ImGui::SameLine();
			if (ImGui::Button("Change##ChangeHoverColor", ImVec2(-1, 0))) {
				ImGui::OpenPopup("Hover Color Picker");
			}

			if (ImGui::BeginPopup("Hover Color Picker")) {
				ImGui::ColorPicker3("Hover Color##HoverColor", glm::value_ptr(v));
				ImGui::EndPopup();
			}

			button->setHoverColor(glm::vec4(v[0], v[1], v[2], 1.0f));
		}

		{
			auto v = button->getClickColor();

			ImGui::Text("Click Color");
			ImGui::SameLine();
			ImGui::ColorButton("Click Color Preview", ImVec4(v[0], v[1], v[2], 1), ImGuiColorEditFlags_NoDragDrop, ImVec2(50, ImGui::GetFrameHeight()));
			ImGui::SameLine();
			if (ImGui::Button("Change##ChangeClickColor", ImVec2(-1, 0))) {
				ImGui::OpenPopup("Click Color Picker");
			}

			if (ImGui::BeginPopup("Click Color Picker")) {
				ImGui::ColorPicker3("Click Color##ClickColor", glm::value_ptr(v));
				ImGui::EndPopup();
			}

			button->setClickColor(glm::vec4(v[0], v[1], v[2], 1.0f));
		}
	}

	case UI_TYPE::IMAGE:
	{
		UI::Image* image = button ? dynamic_cast<UI::Button*>(element)->getImage() : dynamic_cast<UI::Image*>(element);

		ImGui::SeparatorText("Image");
		{
			ImGui::Text("Texture");
			ImGui::SameLine(100);
			auto v = image->getTexture();

			editor->ui->textureSelector("##ImageTextureID", &v);

			image->setTexture(v);
		}

		if (element->getType() != UI_TYPE::BUTTON) break;
	}

	case UI_TYPE::TEXT:
	{
		UI::Text* text = button ? dynamic_cast<UI::Button*>(element)->getTextElement() : dynamic_cast<UI::Text*>(element);

		ImGui::SeparatorText("Text");
		{
			ImGui::Text("Font");
			ImGui::SameLine(100);
			auto v = text->getGlyph();
			editor->ui->textureSelector("##TextGlyphID", &v);

			text->setGlyph(v);
		}

		ImGui::Text("Size");
		ImGui::SameLine(100);
		{
			auto v = text->getTextSize();
			ImGui::InputInt("##UITextSize", &v);

			text->setTextSize(v);
		}

		ImGui::Text("Spacing");
		ImGui::SameLine(100);
		{
			glm::ivec2 v = text->getSpacing();

			ImGui::SetNextItemWidth(90.0f);
			ImGui::InputInt("##TextSpacingX", &v.x);

			ImGui::SameLine();
			ImGui::SetNextItemWidth(90.0f);
			ImGui::InputInt("##TextSpacingY", &v.y);

			text->setSpacing(v);
		}

		ImGui::Text("Centering");
		ImGui::SameLine(100);
		{
			glm::ivec2 v = text->isTextCentered();

			ImGui::SetNextItemWidth(90.0f);
			ImGui::Checkbox("X##TextCenterX", (bool*)&v.x);

			ImGui::SameLine();
			ImGui::SetNextItemWidth(90.0f);
			ImGui::Checkbox("Y##TextCenterY", (bool*)&v.y);

			text->setTextCentered(v);
		}

		{
			auto v = text->getText();

			ImGui::InputTextMultiline("##UITextContent", &v, ImVec2(-1, !button ? 100.0f : 20.0f));

			text->setText(v);
		}
		break;
	}
	}

	ImGui::SeparatorText("Other");

	{
		int selectedElementType = 0;

		ImGui::SetNextItemWidth(-1);
		if (ImGui::Combo("##UIElementType", &selectedElementType, elementTypes.data(), (int)elementTypes.size())) {
			UI::UIElement* child = nullptr;

			switch (selectedElementType) {
			case 1:
				child = element->createChild();
				break;
			case 2:
				child = element->createChild<UI::Image>();
				break;
			case 3:
				child = element->createChild<UI::Text>();
				break;
			case 4:
				child = element->createChild<UI::Button>();
				child->setPivot({ 1, 1 });
				break;
			}

			if (child) {
				child->meta.setMeta("Editor.Name", std::string(elementTypes[selectedElementType]).append(" #" + std::to_string(child->getId())));

				editor->explorerSelection = { EXPLORER_SELECTION_TYPE::UIELEMENT, child->getId() };
			}
		}
	}

	ImGui::Separator();
	ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.5f, 0, 0, 1));
	ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.75f, 0, 0, 1));
	if (ImGui::Button("Delete", ImVec2(-1, 40))) {
		mgr->deleteElement(id);
		editor->explorerSelection = {};
	}
	ImGui::PopStyleColor(2);
}
