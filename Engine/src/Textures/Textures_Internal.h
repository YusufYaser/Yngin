#pragma once
#include <Yngin/Core/Context.h>
#include <Yngin/Renderer/Textures.h>
#include <glad/glad.h>

namespace Yngin {
	struct Texture::Impl {
		Context* ctx;

		uint32_t id;

		GLuint texId;
	};

	struct TexturesManager::Impl {
		Context* ctx;

		std::map<uint32_t, std::unique_ptr<Texture>> textures;
		uint32_t nextId = 0;
	};
}
