#pragma once
#include <Yngin/Core/Context.h>
#include <Yngin/Rendering/Textures.h>
#include <glad/glad.h>
#include <optional>

namespace Yngin {
	constexpr size_t MAX_TEXTURES = std::numeric_limits<uint16_t>::max() + 1;

	struct Texture::Impl {
		Context* ctx;

		uint32_t id;

		GLuint texId;

		glm::ivec2 size = {};

		TextureSettings settings;
	};

	struct TexturesManager::Impl {
		Context* ctx;

		Texture* activeTexture;

		size_t loadedTextures = 0;
		std::unique_ptr<Texture> textures[MAX_TEXTURES];

		std::optional<uint16_t> getAvailableId();
		uint16_t nextId = 0;
		std::vector<uint16_t> deletedIds;
	};
}
