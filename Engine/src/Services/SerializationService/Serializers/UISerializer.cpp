#include <Yngin/Services/SerializationService.h>
#include <Yngin/UI/UI.h>
#include "../SerializationService_Internal.h"
#include "../../Services_Internal.h"
#include "SerializationStructs.h"
#include <sstream>
#include <Yngin/Rendering/Textures.h>

using namespace Yngin::Services::Serialization;
using namespace Yngin::UI;

namespace Yngin::Services {
	bool SerializationService::serialize(std::ostream& out, UIManager* input) {
		return serialize(out, input->getRootElement(), -1);
	}

	bool SerializationService::serialize(std::ostream& out, UIElement* element, int childrenDepth) {
		if (impl->shouldSkip(element->meta)) return true;

		std::stringstream s;
		std::stringstream depend;

		OperationData op{};
		op.schemaVersion = schemaVersion;
		op.op = Operation::UI_ELEMENT;
		op.headerSize = sizeof(GenericUIElementData);

		GenericUIElementData header{};
		header.id = element->getId();
		if (element->getParent()) header.parent = element->getParent()->getId();
		header.position = element->getPosition();
		header.size = element->getSize();
		header.crop = element->getCrop();
		header.color = element->getColor();
		header.pivot = element->getPivot();

		std::vector<UIElement*> children{};

		if (childrenDepth != 0) {
			children = element->getChildren();
			for (auto& child : children) {
				if (!impl->shouldSkip(child->meta)) header.childrenCount++;
			}
		}

		switch (element->getType()) {
		case UI_TYPE::NONE:
		{
			header.type = S_UI_TYPE::NONE;
			header.headerSize = 0;

			s.write(reinterpret_cast<const char*>(&header), op.headerSize);

			break;
		}

		case UI_TYPE::IMAGE:
		{
			header.type = S_UI_TYPE::IMAGE;
			header.headerSize = sizeof(UIImageData);

			s.write(reinterpret_cast<const char*>(&header), op.headerSize);

			Image* image = dynamic_cast<Image*>(element);
			UIImageData imgData{};
			imgData.textureId = image->getTexture();

			s.write(reinterpret_cast<const char*>(&imgData), header.headerSize);

			break;
		}

		case UI_TYPE::TEXT:
		{
			header.type = S_UI_TYPE::TEXT;
			header.headerSize = sizeof(UITextData);

			s.write(reinterpret_cast<const char*>(&header), op.headerSize);

			Text* text = dynamic_cast<Text*>(element);
			UITextData textData{};
			textData.size = text->getTextSize();
			textData.glyphId = text->getGlyph();

			textData.spacing = text->getSpacing();

			textData.centered[0] = text->isTextCentered().x == 1;
			textData.centered[1] = text->isTextCentered().y == 1;

			std::string textString = text->getText();
			textData.textLength = textString.size();

			s.write(reinterpret_cast<const char*>(&textData), header.headerSize);

			s.write(textString.c_str(), textData.textLength);

			break;
		}

		case UI_TYPE::BUTTON:
		{
			header.type = S_UI_TYPE::BUTTON;
			header.headerSize = sizeof(UIButtonData);

			s.write(reinterpret_cast<const char*>(&header), op.headerSize);

			Button* button = dynamic_cast<Button*>(element);
			UIButtonData buttonData{};
			buttonData.hoverColor = button->getHoverColor();
			buttonData.clickColor = button->getClickColor();
			buttonData.imageDataHeaderSize = sizeof(UIImageData);
			buttonData.textDataHeaderSize = sizeof(UITextData);

			s.write(reinterpret_cast<const char*>(&buttonData), header.headerSize);

			{
				Image* image = button->getImage();
				UIImageData imgData{};
				imgData.textureId = image->getTexture();

				s.write(reinterpret_cast<const char*>(&imgData), buttonData.imageDataHeaderSize);

				Text* text = button->getTextElement();
				UITextData textData{};
				textData.size = text->getTextSize();
				textData.glyphId = text->getGlyph();

				textData.spacing = text->getSpacing();

				textData.centered[0] = text->isTextCentered().x == 1;
				textData.centered[1] = text->isTextCentered().y == 1;

				std::string textString = text->getText();
				textData.textLength = textString.size();

				s.write(reinterpret_cast<const char*>(&textData), buttonData.textDataHeaderSize);

				s.write(textString.c_str(), textData.textLength);
			}

			break;
		}

		default:
			return false;
		}


		if (!serialize(s, element->meta)) return false;

		if (!depend.view().empty()) s << depend.rdbuf();

		for (auto& child : children) {
			if (!serialize(s, child, childrenDepth - 1)) return false;
		}

		op.dataSize = s.view().size();

		out.write(reinterpret_cast<const char*>(&op), sizeof(OperationData));
		if (!s.view().empty()) out << s.rdbuf();
		return out.good();
	}

	bool SerializationService::Impl::validateUIElement(std::istream& in, const Serialization::OperationData& op) {
		GenericUIElementData header;

		if (!streamCheck(in, op.headerSize)) return false;
		in.read(reinterpret_cast<char*>(&header), op.headerSize);

		if (header.id == -1) return false;

		switch (header.type) {
		case S_UI_TYPE::NONE:
		{
			if (!streamCheck(in, header.headerSize)) return false;
			in.seekg(header.headerSize, std::ios::cur);
			break;
		}

		case S_UI_TYPE::BUTTON:
			UIButtonData buttonData;
			if (!streamCheck(in, header.headerSize)) return false;
			in.read(reinterpret_cast<char*>(&buttonData), header.headerSize);
			[[fallthrough]];

		case S_UI_TYPE::IMAGE:
		{
			size_t size = header.headerSize;
			if (header.type == S_UI_TYPE::BUTTON) {
				size = buttonData.imageDataHeaderSize;
			}
			if (!streamCheck(in, size)) return false;
			in.seekg(size, std::ios::cur);

			if (header.type != S_UI_TYPE::BUTTON) break;
			[[fallthrough]];
		}

		case S_UI_TYPE::TEXT:
		{
			size_t size = header.headerSize;
			if (header.type == S_UI_TYPE::BUTTON) {
				size = buttonData.textDataHeaderSize;
			}

			UITextData textData;
			if (!streamCheck(in, size)) return false;
			in.read(reinterpret_cast<char*>(&textData), size);

			if (!streamCheck(in, textData.textLength)) return false;
			in.seekg(textData.textLength, std::ios::cur);
			break;
		}

		default:
			return false;
		}

		if (!validateOperation(in, Operation::META)) return false;

		return true;
	}
}
