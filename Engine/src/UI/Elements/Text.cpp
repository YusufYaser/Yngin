#include <Yngin/Core/Context.h>
#include <Yngin/UI/Elements/Text.h>
#include <Yngin/UI/Elements/UIElement.h>
#include <Yngin/Rendering/Shaders.h>
#include "UI_Elements_Internal.h"
#include <glad/glad.h>
#include <Yngin/Core/Models.h>
#include <Yngin/Rendering/Textures.h>
#include "../../Core/Context/Context_Internal.h"

namespace Yngin::UI {
	Text::Text(Context* ctx, Scene* scene, UIManager* mgr, UIElement* parent) : UIElement(ctx, scene, mgr, parent) {
		impl = std::make_unique<Impl>();
		UIElement::impl->pos = {};
		UIElement::impl->size = { 1.0f, 0, 1.0f, 0 };
		UIElement::impl->pivot = {};

		if (mgr) impl->texId = mgr->getDefaultTextGlyph();
	}

	Text::~Text() = default;

	UI_TYPE Text::getType() const {
		return UI_TYPE::TEXT;
	}

	void Text::setText(std::string newText) {
		impl->text = newText;
		impl->textDims = { -1, -1 };
	}

	std::string Text::getText() const {
		return impl->text;
	}

	void Text::setTextSize(int textSize) {
		impl->textSize = textSize;
	}

	int Text::getTextSize() const {
		return impl->textSize;
	}

	void Text::setGlyph(Texture* glyph) {
		if (glyph->getContext() == UIElement::impl->ctx)
			impl->texId = glyph->getId();
	}

	void Text::setGlyph(uint32_t texId) {
		impl->texId = texId;
	}

	uint32_t Text::getGlyph() const {
		return impl->texId;
	}

	void Text::setSpacing(glm::ivec2 spacing) {
		impl->spacing = spacing;
	}

	glm::ivec2 Text::getSpacing() const {
		return impl->spacing;
	}

	glm::ivec2 Text::getTextDimensions() const {
		if (impl->textDims.x != -1) return impl->textDims;

		glm::ivec2 pos = {};
		glm::ivec2 dims = {};
		for (char c : impl->text) {
			if (pos.y == 0) pos.y = 1;
			if (c == '\n') {
				if (pos.x > dims.x) {
					dims.x = pos.x;
				}
				pos.x = 0;
				pos.y++;
				continue;
			}
			if (c == '\r' || c == '\b') {
				continue;
			}
			if (c == ' ') {
				pos.x++;
				continue;
			}
			if (c == '\t') {
				pos.x += 4 - (pos.x % 4);
				continue;
			}

			pos.x++;
		}

		dims = glm::ivec2(
			std::max(pos.x, dims.x),
			std::max(pos.y, 1)
		);

		impl->textDims = dims;

		return dims;
	}

	// TODO: use auto text sizing and centering here
	glm::ivec2 Text::getTextDimensionsPixels() const {
		glm::ivec2 dims = getTextDimensions();
		dims.x *= impl->textSize;
		dims.y *= (int)round(impl->textSize * 1.5);
		return dims;
	}

	void Text::setTextCentered(glm::ivec2 centering) {
		impl->centering = centering;
	}

	glm::ivec2 Text::isTextCentered() {
		return impl->centering;
	}

	void Text::render() {
		UIElement::impl->prepareUniforms();

		Shader* uiShader = UIElement::impl->ctx->getShadersManager()->getShader(SHADER_TYPE::UI);

		Texture* tex = UIElement::impl->ctx->getTexturesManager()->getTexture(impl->texId);
		if (tex) tex->activate();

		glm::ivec2 dimsGrids = getTextDimensions();
		glm::ivec2 dimsPixels = getTextDimensionsPixels();

		glm::ivec2 screenSize = UIElement::impl->ctx->getViewportSize();

		glm::ivec2 maxSize = {
			UIElement::impl->size.xScale * screenSize.x + UIElement::impl->size.xOffset,
			UIElement::impl->size.yScale * screenSize.y + UIElement::impl->size.yOffset
		};

		glm::ivec2 initOffset = {
			UIElement::impl->pos.xOffset - (UIElement::impl->pivot.x * dimsPixels.x),
			UIElement::impl->pos.yOffset - (UIElement::impl->pivot.y * dimsPixels.y)
		};

		int textSize = impl->textSize;

		if (textSize == 0) {
			textSize = maxSize.x / dimsGrids.x - impl->spacing.x;
			textSize = std::min((int)floor((maxSize.y / dimsGrids.y - impl->spacing.y) / 1.5), textSize);

			dimsPixels.x = dimsGrids.x * textSize;
			dimsPixels.y = dimsGrids.y * (int)round(textSize * 1.5);
			initOffset = {
				UIElement::impl->pos.xOffset - (UIElement::impl->pivot.x * dimsPixels.x),
				UIElement::impl->pos.yOffset - (UIElement::impl->pivot.y * dimsPixels.y)
			};
		}

		if (impl->centering.x) {
			initOffset.x += (maxSize.x - dimsPixels.x) / 2;
		}
		if (impl->centering.y) {
			initOffset.y += (maxSize.y - dimsPixels.y) / 2;
		}

		uiShader->setFloat("uiSize.xScale", 0);
		uiShader->setInt("uiSize.xOffset", textSize);
		uiShader->setFloat("uiSize.yScale", 0);
		uiShader->setInt("uiSize.yOffset", textSize * 2);

		glm::ivec2 pos = glm::ivec2(0);

		for (char c : impl->text) {
			if (c == '\n') {
				pos.x = 0;
				pos.y++;
				continue;
			}
			if (c == '\r' || c == '\b') {
				continue;
			}
			if (c == ' ') {
				pos.x++;
				continue;
			}
			if (c == '\t') {
				pos.x += 4 - (pos.x % 4);
				continue;
			}

			glm::ivec2 offset = {
				initOffset.x + int((textSize + impl->spacing.x) * (pos.x + 0.5)),
				initOffset.y + int((textSize + impl->spacing.y) * (pos.y + 0.5) * 1.5)
			};

			if (offset.x >= maxSize.x) continue;
			if (offset.y >= maxSize.y) break;

			glm::vec2 gridPos = glm::vec2(c % 16, c / 16);
			glm::vec2 start = glm::vec2(
				gridPos.x / 16.0f,
				gridPos.y / 16.0f
			);
			glm::vec2 end = glm::vec2(
				(gridPos.x + 1) / 16.0f,
				(gridPos.y + 1) / 16.0f
			);

			uiShader->setVec2("uiCrop.start", start);
			uiShader->setVec2("uiCrop.end", end);

			uiShader->setInt("uiPosition.xOffset", offset.x);
			uiShader->setInt("uiPosition.yOffset", offset.y);

			Model* model = UIElement::impl->ctx->getInternalModelsManager()->getModel(INTERNAL_MODEL_SQUARE_ID);
			glDisable(GL_DEPTH_TEST);
			model->render();
			glEnable(GL_DEPTH_TEST);

			pos.x++;
		}

		UIElement::render();
	}



	void Text::setCrop(UICrop newCrop) {}

	UICrop Text::getCrop() const {
		return {};
	}
}