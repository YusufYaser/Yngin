#include <Yngin/Services/SerializationService.h>
#include <Yngin/UI/UI.h>
#include "../SerializationService_Internal.h"
#include "../../Services_Internal.h"
#include "SerializationStructs.h"
#include <sstream>
#include <Yngin/Rendering/Textures.h>

#define LOGGER_NAME SerializationService
#include "../../../Internal/Logger.h"

using namespace Yngin::Services::Serialization;
using namespace Yngin::UI;

namespace Yngin::Services {
	bool SerializationService::serialize(std::ostream& out, UIManager* input) {
		return serialize(out, input->getRootElement(), -1);
	}

	bool SerializationService::serialize(std::ostream& out, UIElement* element, int childrenDepth) {
		if (impl->shouldSkip(element->meta)) {
			TRACE("Skipping UI element with id %u", element->getId());
			return true;
		}

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

	DESERIALIZATION_STATUS SerializationService::Impl::validateUIElement(std::istream& in, const Serialization::OperationData& op) {
		GenericUIElementData header{};

		if (!streamCheck(in, op.headerSize, sizeof(header))) return DESERIALIZATION_STATUS::INVALID_DATA;
		in.read(reinterpret_cast<char*>(&header), op.headerSize);

		if (header.id == -1) return DESERIALIZATION_STATUS::INVALID_DATA;

		UIButtonData buttonData{};

		switch (header.type) {
		case S_UI_TYPE::NONE:
		{
			if (!streamCheck(in, header.headerSize, sizeof(GenericUIElementData))) return DESERIALIZATION_STATUS::INVALID_DATA;
			in.seekg(header.headerSize, std::ios::cur);
			break;
		}

		case S_UI_TYPE::BUTTON:
			if (!streamCheck(in, header.headerSize, sizeof(buttonData))) return DESERIALIZATION_STATUS::INVALID_DATA;
			in.read(reinterpret_cast<char*>(&buttonData), header.headerSize);
			[[fallthrough]];

		case S_UI_TYPE::IMAGE:
		{
			size_t size = header.headerSize;
			if (header.type == S_UI_TYPE::BUTTON) {
				size = buttonData.imageDataHeaderSize;
			}
			if (!streamCheck(in, size, sizeof(UIImageData))) return DESERIALIZATION_STATUS::INVALID_DATA;
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
			if (!streamCheck(in, size, sizeof(UITextData))) return DESERIALIZATION_STATUS::INVALID_DATA;
			in.read(reinterpret_cast<char*>(&textData), size);

			if (!streamCheck(in, textData.textLength, -1)) return DESERIALIZATION_STATUS::INVALID_DATA;
			in.seekg(textData.textLength, std::ios::cur);
			break;
		}

		default:
			return DESERIALIZATION_STATUS::INVALID_DATA;
		}

		DESERIALIZATION_STATUS metaStatus;
		if ((metaStatus = validateOperation(in, Operation::META)) != DESERIALIZATION_STATUS::OK) return metaStatus;

		for (uint32_t i = 0; i < header.childrenCount; i++) {
			DESERIALIZATION_STATUS status;
			if ((status = validateOperation(in, Operation::UI_ELEMENT)) != DESERIALIZATION_STATUS::OK) return status;
		}

		return DESERIALIZATION_STATUS::OK;
	}

	DESERIALIZATION_STATUS SerializationService::Impl::deserializeUIElement(std::istream& in, const Serialization::OperationData& op, InternalDeserializationContext& dsctx) {
		if (dsctx.UIManager.empty() || dsctx.UIManager.top() == nullptr || dsctx.UIManager.top()->getContext() != ctx) {
			TRACE("Missing UIManager context for UI element deserialization");
			// Skip the operation data in case we ignore missing context errors
			if (!streamCheck(in, op.dataSize, -1)) return DESERIALIZATION_STATUS::GENERIC_ERROR;
			in.seekg(op.dataSize, std::ios::cur);
			return DESERIALIZATION_STATUS::MISSING_CONTEXT;
		}

		UIManager* mgr = dsctx.UIManager.top();

		GenericUIElementData header{};

		if (!streamCheck(in, op.headerSize, sizeof(header))) return DESERIALIZATION_STATUS::INVALID_DATA;
		in.read(reinterpret_cast<char*>(&header), op.headerSize);

		UIElement* element = nullptr;

		UIButtonData buttonData{};

		switch (header.type) {
		case S_UI_TYPE::NONE:
		{
			if (!streamCheck(in, header.headerSize, sizeof(GenericUIElementData))) return DESERIALIZATION_STATUS::INVALID_DATA;
			in.seekg(header.headerSize, std::ios::cur);

			if (header.id == 0) {
				element = mgr->getRootElement();
			} else {
				element = mgr->getElement(header.id);
			}
			break;
		}

		case S_UI_TYPE::BUTTON:
		{
			if (!streamCheck(in, header.headerSize, sizeof(buttonData))) return DESERIALIZATION_STATUS::INVALID_DATA;
			in.read(reinterpret_cast<char*>(&buttonData), header.headerSize);

			Button* button = mgr->getRootElement()->createChild<Button>(header.id, dsctx.user.overrideConflictingId);

			element = button;
			if (element == nullptr) break;

			button->setHoverColor(buttonData.hoverColor);
			button->setClickColor(buttonData.clickColor);

			[[fallthrough]];
		}

		case S_UI_TYPE::IMAGE:
		{
			size_t size = header.headerSize;
			if (header.type == S_UI_TYPE::BUTTON) {
				size = buttonData.imageDataHeaderSize;
			}
			UIImageData imgData{};
			if (!streamCheck(in, size, sizeof(imgData))) return DESERIALIZATION_STATUS::INVALID_DATA;
			in.read(reinterpret_cast<char*>(&imgData), size);

			Image* image = nullptr;
			if (header.type == S_UI_TYPE::BUTTON) {
				Button* button = dynamic_cast<Button*>(element);
				image = button->getImage();
			} else {
				image = mgr->getRootElement()->createChild<Image>(header.id, dsctx.user.overrideConflictingId);
			}

			element = image;
			if (element == nullptr) break;

			image->setTexture(imgData.textureId);

			if (header.type != S_UI_TYPE::BUTTON) break;
			[[fallthrough]];
		}

		case S_UI_TYPE::TEXT:
		{
			size_t size = header.headerSize;
			if (header.type == S_UI_TYPE::BUTTON) {
				size = buttonData.textDataHeaderSize;
			}

			UITextData textData{};
			if (!streamCheck(in, size, sizeof(textData))) return DESERIALIZATION_STATUS::INVALID_DATA;
			in.read(reinterpret_cast<char*>(&textData), size);

			Text* text = nullptr;
			if (header.type == S_UI_TYPE::BUTTON) {
				Button* button = dynamic_cast<Button*>(element);
				text = button->getTextElement();
			} else {
				text = mgr->getRootElement()->createChild<Text>(header.id, dsctx.user.overrideConflictingId);
			}

			element = text;
			if (element == nullptr) break;

			if (!streamCheck(in, textData.textLength, -1)) return DESERIALIZATION_STATUS::INVALID_DATA;
			std::vector<char> textBuffer(textData.textLength);
			in.read(textBuffer.data(), textData.textLength);

			text->setText(std::string(textBuffer.data(), textData.textLength));
			text->setGlyph(textData.glyphId);
			text->setSpacing(textData.spacing);
			text->setTextCentered(glm::ivec2(textData.centered[0], textData.centered[1]));

			break;
		}

		default:
			return DESERIALIZATION_STATUS::INVALID_DATA;
		}

		if (element == nullptr) return DESERIALIZATION_STATUS::GENERIC_ERROR;

		if (header.id != 0) element->setParent(header.parent);
		element->setPosition(header.position);
		element->setSize(header.size);
		element->setCrop(header.crop);
		element->setColor(header.color);
		element->setPivot(header.pivot);

		dsctx.meta.push(&element->meta);
		DESERIALIZATION_STATUS metaStatus = deserializeOperation(in, dsctx, Operation::META);
		dsctx.meta.pop();
		if (metaStatus != DESERIALIZATION_STATUS::OK) return metaStatus;

		for (uint32_t i = 0; i < header.childrenCount; i++) {
			DESERIALIZATION_STATUS status;
			if ((status = deserializeOperation(in, dsctx, Operation::UI_ELEMENT)) != DESERIALIZATION_STATUS::OK) return status;
		}

		return DESERIALIZATION_STATUS::OK;
	}
}
