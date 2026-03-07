#include <Yngin/Core/Context.h>
#include <Yngin/UI/Elements/Text.h>
#include <Yngin/UI/Elements/UIElement.h>
#include <Yngin/Renderer/Shaders.h>
#include "UI_Elements_Internal.h"
#include <glad/glad.h>
#include <Yngin/Core/Models.h>
#include <Yngin/Renderer/Textures.h>

namespace Yngin {
	namespace UI {
		Text::Text(Context* ctx, Scene* scene, UIManager* mgr, UIElement* parent) : UIElement(ctx, scene, mgr, parent) {
			impl = std::make_unique<Impl>();
		}

		Text::~Text() = default;

		UIType Text::getType() {
			return UIType::Text;
		}

		void Text::setText(std::string newText) {
			impl->text = newText;
		}

		std::string Text::getText() {
			return impl->text;
		}

		void Text::setTextSize(int textSize) {
			impl->textSize = textSize;
		}

		int Text::getTextSize() {
			return impl->textSize;
		}

		void Text::setSize(int textSize) {
			impl->textSize = textSize;
		}

		int Text::getSize() {
			return impl->textSize;
		}

		void Text::setGlyph(Texture* glyph) {
			impl->texId = glyph->getId();
		}

		void Text::setGlyph(uint32_t texId) {
			impl->texId = texId;
		}

		uint32_t Text::getGlyph() {
			return impl->texId;
		}

		void Text::render() {
			UIElement::impl->prepareUniforms();

			Shader* uiShader = UIElement::impl->ctx->getShadersManager()->getShader(SHADER_TYPE::UI);

			Texture* tex = UIElement::impl->ctx->getTexturesManager()->getTexture(impl->texId);
			if (tex) tex->activate();

			uiShader->setFloat("uiSize.xScale", 0);
			uiShader->setInt("uiSize.xOffset", impl->textSize);
			uiShader->setFloat("uiSize.yScale", 0);
			uiShader->setInt("uiSize.yOffset", impl->textSize * 2);

			glm::ivec2 pos = glm::ivec2(0);

			for (char c : impl->text) {
				if (c == '\n') {
					pos.x = 0;
					pos.y++;
					continue;
				}
				if (c == '\r') {
					pos.x = 0;
					continue;
				}

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

				uiShader->setInt("uiPosition.xOffset", UIElement::impl->pos.xOffset + impl->textSize * pos.x);
				uiShader->setInt("uiPosition.yOffset", UIElement::impl->pos.xOffset + impl->textSize * 2 * pos.y);

				Model* model = UIElement::impl->ctx->getImageModel();
				glDisable(GL_DEPTH_TEST);
				model->render();
				glEnable(GL_DEPTH_TEST);

				pos.x++;
			}

			UIElement::render();
		}



		void Text::setCrop(UICrop newCrop) {}

		UICrop Text::getCrop() {
			return {};
		}

		void Text::setPivot(glm::vec2 newPivot) {}

		glm::vec2 Text::getPivot() {
			return {};
		}
	}
}