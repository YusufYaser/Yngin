#include <Yngin/Rendering/Textures.h>
#include "Textures_Internal.h"
#include <glad/glad.h>
#include <stdexcept>
#include <stb/stb_image.h>


namespace Yngin {
	namespace {
		GLint texFilterToGlFilter(const TEXTURE_FILTER& filter) {
			GLint glFilter = GL_LINEAR;
			switch (filter) {
			case TEXTURE_FILTER::NEAREST:
				glFilter = GL_NEAREST;
				break;
			case TEXTURE_FILTER::LINEAR:
				glFilter = GL_LINEAR;
				break;
			case TEXTURE_FILTER::NEAREST_MIPMAP_NEAREST:
				glFilter = GL_NEAREST_MIPMAP_NEAREST;
				break;
			case TEXTURE_FILTER::LINEAR_MIPMAP_NEAREST:
				glFilter = GL_LINEAR_MIPMAP_NEAREST;
				break;
			case TEXTURE_FILTER::NEAREST_MIPMAP_LINEAR:
				glFilter = GL_NEAREST_MIPMAP_LINEAR;
				break;
			case TEXTURE_FILTER::LINEAR_MIPMAP_LINEAR:
				glFilter = GL_LINEAR_MIPMAP_LINEAR;
				break;
			}

			return glFilter;
		}
	}

	Texture::Texture(Context* ctx) {
		impl = std::make_unique<Impl>();
		impl->ctx = ctx;
	}

	Texture::~Texture() {
		glDeleteTextures(1, &impl->texId);
	}

	uint16_t Texture::getId() const {
		return impl->id;
	}

	Context* Texture::getContext() const {
		return impl->ctx;
	}

	uint32_t Texture::getGLid() const {
		return impl->texId;
	}

	void Texture::activate() {
		glBindTexture(GL_TEXTURE_2D, impl->texId);
	}

	void Texture::setData(const TextureData& data) {
		setData(data, impl->settings);
	}

	void Texture::setData(const char* path) {
		setData(path, impl->settings);
	}

	void Texture::setData(const TextureData& data, const TextureSettings& settings) {
		int w = data.width;
		int h = data.height;
		int n = data.numCh;

		const char* bytes = data.bytes;

		Texture* active = impl->ctx->getTexturesManager()->getActive();
		if (active)	glBindTexture(GL_TEXTURE_2D, 0);

		glGenTextures(1, &impl->texId);
		glBindTexture(GL_TEXTURE_2D, impl->texId);

		char* finalBytes = new char[w * h * 4 + 1];
		int cIdx = 0;

		// convert to RGBA
		if (n == 1) {
			for (int i = 0; i < w * h; i++) {
				finalBytes[cIdx++] = bytes[i * n];
				finalBytes[cIdx++] = bytes[i * n];
				finalBytes[cIdx++] = bytes[i * n];
				finalBytes[cIdx++] = '\xff';
			}
			finalBytes[cIdx] += '\0';
		} else if (n == 2) {
			for (int i = 0; i < w * h; i++) {
				finalBytes[cIdx++] = bytes[i * n];
				finalBytes[cIdx++] = bytes[i * n];
				finalBytes[cIdx++] = bytes[i * n];
				finalBytes[cIdx++] = bytes[i * n + 1];
			}
			finalBytes[cIdx] += '\0';
		} else if (n == 3) {
			for (int i = 0; i < w * h; i++) {
				finalBytes[cIdx++] = bytes[i * n];
				finalBytes[cIdx++] = bytes[i * n + 1];
				finalBytes[cIdx++] = bytes[i * n + 2];
				finalBytes[cIdx++] = '\xff';
			}
			finalBytes[cIdx] += '\0';
		} else if (n > 4 || n < 1) {
			throw std::invalid_argument("Number of channels cannot be more than 4 or less than 1");
		} else {
			delete[] finalBytes;
			finalBytes = (char*)bytes;
		}

		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, finalBytes);
		glGenerateMipmap(GL_TEXTURE_2D);

		if (n != 4) {
			delete[] finalBytes;
		} else {
			finalBytes = nullptr;
		}

		if (active) {
			impl->ctx->getTexturesManager()->setActive(active->getId());
		} else {
			impl->ctx->getTexturesManager()->setActive(0);
		}

		impl->size = glm::ivec2(w, h);
		setSettings(settings);
	}

	void Texture::setData(const char* path, const TextureSettings& settings) {
		TextureData data{};
		unsigned char* bytes = stbi_load(path, &data.width, &data.height, &data.numCh, 0);
		if (!bytes) return;

		data.bytes = (const char*)bytes;

		setData(data, settings);

		stbi_image_free(bytes);
	}

	void Texture::setSettings(const TextureSettings& settings) {
		Texture* active = impl->ctx->getTexturesManager()->getActive();

		activate();

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, texFilterToGlFilter(settings.filterMin));
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, texFilterToGlFilter(settings.filterMag));

		GLint glWrap = GL_REPEAT;
		switch (settings.wrap) {
		case TEXTURE_WRAP::REPEAT:
			glWrap = GL_REPEAT;
			break;
		case TEXTURE_WRAP::CLAMP:
			glWrap = GL_CLAMP_TO_EDGE;
			break;
		}

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, glWrap);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, glWrap);

		impl->settings = settings;

		if (active) {
			impl->ctx->getTexturesManager()->setActive(active->getId());
		} else {
			impl->ctx->getTexturesManager()->setActive(0);
		}
	}

	glm::ivec2 Texture::getSize() const {
		return impl->size;
	}

	const TextureSettings& Texture::getTextureSettings() {
		return impl->settings;
	}
}
