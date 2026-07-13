#include <Yngin/Rendering/Textures.h>
#include "Textures_Internal.h"
#include <stdexcept>
#include <glad/glad.h>

#define LOGGER_NAME Textures
#include "../../Internal/Logger.h"

namespace Yngin {
	TexturesManager::TexturesManager(Context* ctx) {
		impl = std::make_unique<Impl>();

		impl->ctx = ctx;

		glActiveTexture(GL_TEXTURE0);
	}

	TexturesManager::~TexturesManager() = default;

	Context* TexturesManager::getContext() const {
		return impl->ctx;
	}

	std::optional<uint16_t> TexturesManager::Impl::getAvailableId() {
		if (loadedTextures >= MAX_TEXTURES) return std::nullopt;

		uint16_t id = nextId;
		while (textures[id] && !deletedIds.empty()) {
			id = deletedIds.back();
			deletedIds.pop_back();
		}

		while (textures[id] && nextId++ < MAX_TEXTURES) {
			id = nextId;
			if (nextId >= MAX_TEXTURES - 1) {
				nextId = 0;
				break;
			}
		}

		if (textures[id]) return std::nullopt;

		return id;
	}

	Texture* TexturesManager::createTexture() {
		auto id = impl->getAvailableId();
		if (!id.has_value()) return nullptr;

		return createTexture(id.value());
	}

	Texture* TexturesManager::createTexture(const TextureData& data, const TextureSettings& settings) {
		auto id = impl->getAvailableId();
		if (!id.has_value()) return nullptr;

		return createTexture(data, settings, id.value());
	}

	Texture* TexturesManager::createTexture(const char* path, const TextureSettings& settings) {
		auto id = impl->getAvailableId();
		if (!id.has_value()) return nullptr;

		return createTexture(path, settings, id.value());
	}

	Texture* TexturesManager::createTexture(uint16_t id, bool override) {
		if (getTexture(id) != nullptr) {
			if (override) {
				// We aren't using deleteTexture() in case we want to override a locked ID (0, 1, or 2)
				impl->textures[id].reset();
				impl->loadedTextures--;
			} else {
				return nullptr;
			}
		}

		auto texture = std::unique_ptr<Texture>(new Texture(impl->ctx));

		if (id == impl->nextId) impl->nextId++;
		texture->impl->id = id;
		impl->textures[id] = std::move(texture);
		impl->loadedTextures++;

		DEBUG("Created texture %d", id);

		return impl->textures[id].get();
	}

	Texture* TexturesManager::createTexture(const TextureData& data, const TextureSettings& settings, uint16_t id, bool override) {
		Texture* tex = createTexture(id, override);
		if (tex) tex->setData(data, settings);
		return tex;
	}

	Texture* TexturesManager::createTexture(const char* path, const TextureSettings& settings, uint16_t id, bool override) {
		Texture* tex = createTexture(id, override);
		if (tex) tex->setData(path, settings);
		return tex;
	}

	void TexturesManager::deleteTexture(uint16_t id) {
		assert(id != 0 && id != 1 && id != 2);

		if (id == 0 || id == 1 || id == 2) return;

		if (!impl->textures[id]) return;

		impl->textures[id].reset();
		impl->loadedTextures--;
		impl->deletedIds.push_back(id);

		DEBUG("Deleted texture %d", id);
	}

	size_t TexturesManager::getMaxTexturesCount() const {
		return MAX_TEXTURES;
	}

	size_t TexturesManager::getTexturesCount() const {
		return impl->loadedTextures;
	}

	std::vector<Texture*> TexturesManager::getTextures() const {
		std::vector<Texture*> textures;
		for (auto& texture : impl->textures) {
			if (texture) textures.push_back(texture.get());
		}
		return textures;
	}

	Texture* TexturesManager::getTexture(uint16_t id) const {
		return impl->textures[id].get();
	}

	Texture* TexturesManager::getActive() const {
		return impl->activeTexture;
	}

	void TexturesManager::setActive(uint16_t textureId) {
		Texture* texture = getTexture(textureId);
		if (texture) texture->activate();
	}
}
