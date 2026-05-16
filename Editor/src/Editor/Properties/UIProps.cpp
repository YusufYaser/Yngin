#include <Yngin/UI/UI.h>
#include <ImGui/imgui.h>
#include "../Editor.h"
#include <string>
#include "../UI/UI.h"

using namespace Yngin;

void Editor::showUIElementProps(uint32_t id, bool global) {
	UI::UIManager* mgr = activeScene->getUIManager();
	if (global) mgr = ctx->getGlobalUIManager();

	static std::vector<const char*> elementTypes = {
		"Element",
		"Image",
		"Text",
		"Button"
	};

	if (id == -1) {
		ImGui::Text("Scene UI Elements");

		static int selectedElementType = 0;
		ImGui::SetNextItemWidth(-1);
		ImGui::Combo("##UIElementType", &selectedElementType, elementTypes.data(), (int)elementTypes.size());

		if (ImGui::Button("Create UI Element", ImVec2(-1, 40))) {
			UI::UIElement* child = nullptr;

			switch (selectedElementType) {
			case 0:
				child = mgr->getRootElement()->createChild();
				break;
			case 1:
				child = mgr->getRootElement()->createChild<UI::Image>();
				break;
			case 2:
				child = mgr->getRootElement()->createChild<UI::Text>();
				break;
			case 3:
				child = mgr->getRootElement()->createChild<UI::Button>();
				child->setPivot({ 1, 1 });
				break;
			}
			child->meta.setMeta("Editor.Name", std::string(elementTypes[selectedElementType]).append(" #" + std::to_string(child->getId())));

			if (child) explorerSelection = { EXPLORER_SELECTION_TYPE::UIELEMENT, child->getId() };
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
		static char v[32] = {};
		ImGui::Text("Name");
		ImGui::SameLine(100);
		if (ImGui::InputText("##UIName", v, 32)) {
			name = v;
			element->meta.setMeta("Editor.Name", name);
		} else {
			strcpy_s(v, 32, name.c_str());
		}
	}

	ImGui::SeparatorText("Position");
	{
		UI::UITransform pos = element->getPosition();
		static UI::UITransform v = {};

		ImGui::Text("Scale");
		ImGui::SameLine(100);

		ImGui::SetNextItemWidth(90.0f);
		if (ImGui::InputFloat("##PosXScale", &v.xScale, 0.05f, 0.05f, "%.2f")) {
			pos.xScale = v.xScale;
		} else {
			v.xScale = pos.xScale;
		}
		ImGui::SameLine();
		ImGui::SetNextItemWidth(90.0f);
		if (ImGui::InputFloat("##PosYScale", &v.yScale, 0.05f, 0.05f, "%.2f")) {
			pos.yScale = v.yScale;
		} else {
			v.yScale = pos.yScale;
		}


		ImGui::Text("Offset");
		ImGui::SameLine(100);

		ImGui::SetNextItemWidth(90.0f);
		if (ImGui::InputInt("##PosXOffset", &v.xOffset)) {
			pos.xOffset = v.xOffset;
		} else {
			v.xOffset = pos.xOffset;
		}
		ImGui::SameLine();
		ImGui::SetNextItemWidth(90.0f);
		if (ImGui::InputInt("##PosYOffset", &v.yOffset)) {
			pos.yOffset = v.yOffset;
		} else {
			v.yOffset = pos.yOffset;
		}



		element->setPosition(pos);
	}

	ImGui::SeparatorText("Pivot");
	{
		glm::vec2 pivot = element->getPivot();
		static glm::vec2 v = {};

		ImGui::SetNextItemWidth(136.0f);
		if (ImGui::InputFloat("##PivotX", &v.x, 0.05f, 0.05f, "%.2f")) {
			pivot.x = v.x;
		} else {
			v.x = pivot.x;
		}
		ImGui::SameLine();
		ImGui::SetNextItemWidth(136.0f);
		if (ImGui::InputFloat("##PivotY", &v.y, 0.05f, 0.05f, "%.2f")) {
			pivot.y = v.y;
		} else {
			v.y = pivot.y;
		}

		element->setPivot(pivot);
	}

	ImGui::SeparatorText("Size");
	{
		UI::UITransform size = element->getSize();
		static UI::UITransform v = {};

		ImGui::Text("Scale");
		ImGui::SameLine(100);

		ImGui::SetNextItemWidth(90.0f);
		if (ImGui::InputFloat("##SizeXScale", &v.xScale, 0.05f, 0.05f, "%.2f")) {
			size.xScale = v.xScale;
		} else {
			v.xScale = size.xScale;
		}
		ImGui::SameLine();
		ImGui::SetNextItemWidth(90.0f);
		if (ImGui::InputFloat("##SizeYScale", &v.yScale, 0.05f, 0.05f, "%.2f")) {
			size.yScale = v.yScale;
		} else {
			v.yScale = size.yScale;
		}


		ImGui::Text("Offset");
		ImGui::SameLine(100);

		ImGui::SetNextItemWidth(90.0f);
		if (ImGui::InputInt("##SizeXOffset", &v.xOffset)) {
			size.xOffset = v.xOffset;
		} else {
			v.xOffset = size.xOffset;
		}
		ImGui::SameLine();
		ImGui::SetNextItemWidth(90.0f);
		if (ImGui::InputInt("##SizeYOffset", &v.yOffset)) {
			size.yOffset = v.yOffset;
		} else {
			v.yOffset = size.yOffset;
		}



		element->setSize(size);
	}

	ImGui::SeparatorText("Color");
	{
		static float v[3] = {};
		ImGui::Text("Color");
		ImGui::SameLine();
		ImGui::ColorButton("UI Element Color Preview", ImVec4(v[0], v[1], v[2], 1), ImGuiColorEditFlags_NoDragDrop, ImVec2(50, ImGui::GetFrameHeight()));
		ImGui::SameLine();
		if (ImGui::Button("Change Color##UIColorButton", ImVec2(-1, 0))) {
			ImGui::OpenPopup("UI Element Color Picker");
		}

		if (ImGui::BeginPopup("UI Element Color Picker")) {
			if (ImGui::ColorPicker3("Color##UIColor", v)) {
				element->setColor(glm::vec4(v[0], v[1], v[2], 1.0f));
			}
			ImGui::EndPopup();
		} else {
			v[0] = element->getColor()[0];
			v[1] = element->getColor()[1];
			v[2] = element->getColor()[2];
		}
	}


	if (element->getType() != UI_TYPE::TEXT) {
		ImGui::SeparatorText("Crop");
		{
			UI::UICrop crop = element->getCrop();
			static UI::UICrop v = {};

			ImGui::Text("Start");
			ImGui::SameLine(100);

			ImGui::SetNextItemWidth(90.0f);
			if (ImGui::InputFloat("##CropStartX", &v.start.x, 0.05f, 0.05f)) {
				crop.start.x = v.start.x;
			} else {
				v.start.x = crop.start.x;
			}
			ImGui::SameLine();
			ImGui::SetNextItemWidth(90.0f);
			if (ImGui::InputFloat("##CropStartY", &v.start.y, 0.05f, 0.05f)) {
				crop.start.y = v.start.y;
			} else {
				v.start.y = crop.start.y;
			}


			ImGui::Text("End");
			ImGui::SameLine(100);

			ImGui::SetNextItemWidth(90.0f);
			if (ImGui::InputFloat("##CropEndX", &v.end.x, 0.05f, 0.05f)) {
				crop.end.x = v.end.x;
			} else {
				v.end.x = crop.end.x;
			}
			ImGui::SameLine();
			ImGui::SetNextItemWidth(90.0f);
			if (ImGui::InputFloat("##CropEndY", &v.end.y, 0.05f, 0.05f)) {
				crop.end.y = v.end.y;
			} else {
				v.end.y = crop.end.y;
			}

			element->setCrop(crop);
		}
	}


	bool button = element->getType() == UI_TYPE::BUTTON;
	switch (element->getType()) {
	case UI_TYPE::BUTTON:
	{
		UI::Button* button = dynamic_cast<UI::Button*>(element);

		ImGui::SeparatorText("Button");
		{
			static float v[3] = {};
			ImGui::Text("Hover Color");
			ImGui::SameLine();
			ImGui::ColorButton("Hover Color Preview", ImVec4(v[0], v[1], v[2], 1), ImGuiColorEditFlags_NoDragDrop, ImVec2(50, ImGui::GetFrameHeight()));
			ImGui::SameLine();
			if (ImGui::Button("Change##ChangeHoverColor", ImVec2(-1, 0))) {
				ImGui::OpenPopup("Hover Color Picker");
			}

			if (ImGui::BeginPopup("Hover Color Picker")) {
				if (ImGui::ColorPicker3("Hover Color##HoverColor", v)) {
					button->setHoverColor(glm::vec4(v[0], v[1], v[2], 1.0f));
				}
				ImGui::EndPopup();
			} else {
				v[0] = button->getHoverColor()[0];
				v[1] = button->getHoverColor()[1];
				v[2] = button->getHoverColor()[2];
			}
		}

		{
			static float v[3] = {};
			ImGui::Text("Click Color");
			ImGui::SameLine();
			ImGui::ColorButton("Click Color Preview", ImVec4(v[0], v[1], v[2], 1), ImGuiColorEditFlags_NoDragDrop, ImVec2(50, ImGui::GetFrameHeight()));
			ImGui::SameLine();
			if (ImGui::Button("Change##ChangeClickColor", ImVec2(-1, 0))) {
				ImGui::OpenPopup("Click Color Picker");
			}

			if (ImGui::BeginPopup("Click Color Picker")) {
				if (ImGui::ColorPicker3("Click Color##ClickColor", v)) {
					button->setClickColor(glm::vec4(v[0], v[1], v[2], 1.0f));
				}
				ImGui::EndPopup();
			} else {
				v[0] = button->getClickColor()[0];
				v[1] = button->getClickColor()[1];
				v[2] = button->getClickColor()[2];
			}
		}
	}

	case UI_TYPE::IMAGE:
	{
		UI::Image* image = button ? dynamic_cast<UI::Button*>(element)->getImage() : dynamic_cast<UI::Image*>(element);

		ImGui::SeparatorText("Image");
		{
			ImGui::Text("Texture");
			ImGui::SameLine(100);
			static uint32_t v = 0;
			if (ui->textureSelector("##ImageTextureID", &v)) {
				image->setTexture(v);
			} else {
				v = image->getTexture();
			}
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
			static uint32_t v = 0;
			if (ui->textureSelector("##TextGlyphID", &v)) {
				text->setGlyph(v);
			} else {
				v = text->getGlyph();
			}
		}

		ImGui::Text("Size");
		ImGui::SameLine(100);
		{
			static int v = 0;
			if (ImGui::InputInt("##UITextSize", &v)) {
				text->setTextSize(v);
			} else {
				v = text->getTextSize();
			}
		}

		ImGui::Text("Spacing");
		ImGui::SameLine(100);
		{
			glm::ivec2 spacing = text->getSpacing();
			static glm::ivec2 v = {};

			ImGui::SetNextItemWidth(90.0f);
			if (ImGui::InputInt("##TextSpacingX", &v.x)) {
				spacing.x = v.x;
			} else {
				v.x = spacing.x;
			}
			ImGui::SameLine();
			ImGui::SetNextItemWidth(90.0f);
			if (ImGui::InputInt("##TextSpacingY", &v.y)) {
				spacing.y = v.y;
			} else {
				v.y = spacing.y;
			}

			text->setSpacing(spacing);
		}

		ImGui::Text("Centering");
		ImGui::SameLine(100);
		{
			glm::ivec2 center = text->isTextCentered();
			static glm::ivec2 v = {};

			ImGui::SetNextItemWidth(90.0f);
			if (ImGui::Checkbox("X##TextCenterX", (bool*)&v.x)) {
				center.x = v.x;
			} else {
				v.x = center.x;
			}
			ImGui::SameLine();
			ImGui::SetNextItemWidth(90.0f);
			if (ImGui::Checkbox("Y##TextCenterY", (bool*)&v.y)) {
				center.y = v.y;
			} else {
				v.y = center.y;
			}

			text->setTextCentered(center);
		}

		{
			static char v[1024] = {};
			if (ImGui::InputTextMultiline("##UITextContent", v, 1024, ImVec2(-1, !button ? 100.0f : 20.0f))) {
				text->setText(v);
			} else {
				strcpy_s(v, 1024, text->getText().c_str());
			}
		}
		break;
	}
	}

	ImGui::SeparatorText("Other");

	{
		static int selectedElementType = 0;
		ImGui::Combo("##UIElementType", &selectedElementType, elementTypes.data(), (int)elementTypes.size());

		ImGui::SameLine();
		if (ImGui::Button("New Child", ImVec2(-1, 0))) {
			UI::UIElement* child = nullptr;

			switch (selectedElementType) {
			case 0:
				child = element->createChild();
				break;
			case 1:
				child = element->createChild<UI::Image>();
				break;
			case 2:
				child = element->createChild<UI::Text>();
				break;
			case 3:
				child = element->createChild<UI::Button>();
				child->setPivot({ 1, 1 });
				break;
			}
			child->meta.setMeta("Editor.Name", std::string(elementTypes[selectedElementType]).append(" #" + std::to_string(child->getId())));

			if (child) explorerSelection = { EXPLORER_SELECTION_TYPE::UIELEMENT, child->getId() };
		}
	}

	ImGui::Separator();
	ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.5f, 0, 0, 1));
	ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.75f, 0, 0, 1));
	if (ImGui::Button("Delete", ImVec2(-1, 40))) {
		mgr->deleteElement(id);
		explorerSelection = {};
	}
	ImGui::PopStyleColor(2);
}
