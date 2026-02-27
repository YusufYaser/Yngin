#include <Yngin/Renderer/Textures.h>
#include "Textures_Internal.h"
#include <stdexcept>
#include <glad/glad.h>

namespace Yngin {
	TexturesManager::TexturesManager(Context* ctx) {
		if (ctx->getTexturesManager()) {
			throw std::invalid_argument("Context already has a textures manager!");
		}

		impl = std::make_unique<Impl>();

		impl->ctx = ctx;

		glActiveTexture(GL_TEXTURE0);
	}

	TexturesManager::~TexturesManager() = default;

	uint32_t TexturesManager::createTexture() {
		auto texture = std::unique_ptr<Texture>(new Texture(impl->ctx));

		uint32_t textureId = impl->nextId++;
		impl->textures[textureId] = std::move(texture);
		return textureId;
	}

	uint32_t TexturesManager::createTexture(TextureData& data) {
		uint32_t tex = createTexture();
		getTexture(tex)->setData(data);
		return tex;
	}

	void TexturesManager::deleteTexture(uint32_t textureId) {
		assert(textureId != 0);

		if (textureId == 0) return;

		impl->textures.erase(textureId);
	}

	Texture* TexturesManager::getTexture(uint32_t textureId) {
		auto it = impl->textures.find(textureId);
		assert(it != impl->textures.end());

		if (it == impl->textures.end()) return nullptr;

		return it->second.get();
	}

	Texture* TexturesManager::getActive() {
		return impl->activeTexture;
	}

	void TexturesManager::setActive(uint32_t textureId) {
		Texture* texture = getTexture(textureId);
		texture->activate();
	}
}
