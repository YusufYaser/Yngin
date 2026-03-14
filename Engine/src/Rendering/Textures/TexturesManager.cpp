#include <Yngin/Rendering/Textures.h>
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

	Texture* TexturesManager::createTexture() {
		auto texture = std::unique_ptr<Texture>(new Texture(impl->ctx));

		uint32_t textureId = impl->nextId++;
		texture->impl->id = textureId;
		impl->textures[textureId] = std::move(texture);

		return impl->textures[textureId].get();
	}

	Texture* TexturesManager::createTexture(const TextureData& data, const TextureSettings& settings) {
		Texture* tex = createTexture();
		tex->setData(data, settings);
		return tex;
	}

	Texture* TexturesManager::createTexture(const char* path, const TextureSettings& settings) {
		Texture* tex = createTexture();
		tex->setData(path, settings);
		return tex;
	}

	void TexturesManager::deleteTexture(uint32_t textureId) {
		assert(textureId != 0);

		if (textureId == 0) return;

		impl->textures.erase(textureId);
	}

	size_t TexturesManager::getTexturesCount() const {
		return impl->textures.size();
	}

	std::vector<Texture*> TexturesManager::getTextures() const {
		std::vector<Texture*> textures;
		for (auto& kvp : impl->textures) {
			textures.push_back(kvp.second.get());
		}
		return textures;
	}

	Texture* TexturesManager::getTexture(uint32_t textureId) const {
		auto it = impl->textures.find(textureId);
		if (it == impl->textures.end()) return nullptr;

		return it->second.get();
	}

	Texture* TexturesManager::getActive() const {
		return impl->activeTexture;
	}

	void TexturesManager::setActive(uint32_t textureId) {
		Texture* texture = getTexture(textureId);
		texture->activate();
	}
}
