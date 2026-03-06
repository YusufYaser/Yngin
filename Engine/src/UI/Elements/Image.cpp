#include <Yngin/UI/Elements/Image.h>
#include <Yngin/UI/Elements/UIElement.h>
#include <Yngin/Renderer/Shaders.h>
#include <Yngin/Core/Context.h>
#include <Yngin/Core/Models.h>
#include <Yngin/Renderer/Textures.h>
#include "UI_Elements_Internal.h"
#include <glad/glad.h>

namespace Yngin {
	namespace UI {
		Image::Image(Context* ctx, Scene* scene, UIManager* mgr, UIElement* parent) : UIElement(ctx, scene, mgr, parent) {
			impl = std::make_unique<Impl>();
		}

		Image::~Image() = default;

		UIType Image::getType() {
			return UIType::Image;
		}

		void Image::render() {
			Shader* uiShader = UIElement::impl->ctx->getShadersManager()->getShader(SHADER_TYPE::UI);
			uiShader->activate();

			uiShader->setFloat("uiPosition.xScale", UIElement::impl->pos.xScale);
			uiShader->setInt("uiPosition.xOffset", UIElement::impl->pos.xOffset);
			uiShader->setFloat("uiPosition.yScale", UIElement::impl->pos.yScale);
			uiShader->setInt("uiPosition.yOffset", UIElement::impl->pos.yOffset);

			uiShader->setFloat("uiSize.xScale", UIElement::impl->size.xScale);
			uiShader->setInt("uiSize.xOffset", UIElement::impl->size.xOffset);
			uiShader->setFloat("uiSize.yScale", UIElement::impl->size.yScale);
			uiShader->setInt("uiSize.yOffset", UIElement::impl->size.yOffset);

			uiShader->setIVec2("screenSize", UIElement::impl->ctx->getViewportSize());

			uiShader->setVec4("color", impl->color);

			UIElement::impl->ctx->getTexturesManager()->getTexture(impl->texId)->activate();

			Model* model = UIElement::impl->ctx->getImageModel();
			glDisable(GL_DEPTH_TEST);
			model->render();
			glEnable(GL_DEPTH_TEST);

			UIElement::render();
		}

		void Image::setTexture(uint32_t newTextureId) {
			impl->texId = newTextureId;
		}

		void Image::setTexture(Texture* newTexture) {
			if (newTexture->getContext() == UIElement::impl->ctx) {
				impl->texId = newTexture->getId();
			}
		}

		uint32_t Image::getTexture() {
			return impl->texId;
		}

		void Image::setColor(glm::vec4 newColor) {
			impl->color = newColor;
		}

		glm::vec4 Image::getColor() {
			return impl->color;
		}
	}
}
