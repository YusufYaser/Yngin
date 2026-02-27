#include <Yngin/Renderer/Textures.h>
#include "Textures_Internal.h"
#include <glad/glad.h>
#include <stdexcept>

namespace Yngin {
	Texture::Texture(Context* ctx) {
		impl = std::make_unique<Impl>();
		impl->ctx = ctx;
	}

	Texture::~Texture() {
		glDeleteTextures(1, &impl->texId);
	}

	void Texture::activate() {
		glBindTexture(GL_TEXTURE_2D, impl->texId);
	}

	void Texture::setData(TextureData& texData) {
		int w = texData.width;
		int h = texData.height;
		int n = texData.numCh;

		const char* data = texData.data;

		bool active = impl->ctx->getTexturesManager()->getActive() == this;
		if (active)	glBindTexture(GL_TEXTURE_2D, 0);

		glGenTextures(1, &impl->texId);
		glBindTexture(GL_TEXTURE_2D, impl->texId);

		GLint glFilter;
		switch (texData.filter) {
		case TEXTURE_FILTER::LINEAR:
			glFilter = GL_LINEAR;
			break;
		case TEXTURE_FILTER::NEAREST:
			glFilter = GL_NEAREST;
			break;
		}

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, glFilter);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, glFilter);

		GLint glWrap;
		switch (texData.wrap) {
		case TEXTURE_WRAP::REPEAT:
			glWrap = GL_REPEAT;
			break;
		case TEXTURE_WRAP::CLAMP:
			glWrap = GL_CLAMP_TO_EDGE;
			break;
		}

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, glWrap);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, glWrap);

		std::string finalData = "";

		// convert to RGBA
		if (n == 1) {
			for (int i = 0; i < w * h; i++) {
				finalData += data[i * n];
				finalData += data[i * n];
				finalData += data[i * n];
				finalData += '\xff';
			}
		} else if (n == 2) {
			for (int i = 0; i < w * h; i++) {
				finalData += data[i * n];
				finalData += data[i * n];
				finalData += data[i * n];
				finalData += data[i * n + 1];
			}
		} else if (n == 3) {
			for (int i = 0; i < w * h; i++) {
				finalData += data[i * n];
				finalData += data[i * n + 1];
				finalData += data[i * n + 2];
				finalData += '\xff';
			}
		} else if (n > 4) {
			throw std::invalid_argument("Number of channels cannot be more than 4");
		} else {
			finalData = data;
		}

		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, finalData.c_str());
		glGenerateMipmap(GL_TEXTURE_2D);

		if (active)	glBindTexture(GL_TEXTURE_2D, impl->texId);
	}
}
