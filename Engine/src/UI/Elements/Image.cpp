#include <Yngin/UI/Elements/Image.h>
#include <Yngin/UI/Elements/UIElement.h>
#include <Yngin/Rendering/Shaders.h>
#include <Yngin/Core/Context.h>
#include <Yngin/Core/Models.h>
#include <Yngin/Rendering/Textures.h>
#include "UI_Elements_Internal.h"
#include <glad/glad.h>
#include "../../Core/Context/Context_Internal.h"

namespace Yngin::UI {
	Image::Image(Context* ctx, Scene* scene, UIManager* mgr, UIElement* parent) : UIElement(ctx, scene, mgr, parent) {
		impl = std::make_unique<Impl>();
	}

	Image::~Image() = default;

	UI_TYPE Image::getType() const {
		return UI_TYPE::IMAGE;
	}

	void Image::render() {
		UIElement::impl->prepareUniforms();

		Texture* tex = UIElement::impl->ctx->getTexturesManager()->getTexture(impl->texId);
		if (tex) tex->activate();

		Model* model = UIElement::impl->ctx->getInternalModelsManager()->getModel(INTERNAL_MODEL_SQUARE_ID);
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

	uint32_t Image::getTexture() const {
		return impl->texId;
	}
}
