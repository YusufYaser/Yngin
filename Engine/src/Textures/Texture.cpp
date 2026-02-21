#include <Yngin/Textures.h>
#include "Textures_Internal.h"
#include <glad/glad.h>
#include <stdexcept>

namespace Yngin {
	Texture::Texture(Context* ctx, int width, int height, int n, const char* data) {
		impl = std::make_unique<Impl>();
		impl->ctx = ctx;

		glGenTextures(1, &impl->texId);
		glBindTexture(GL_TEXTURE_2D, impl->texId);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

		std::string finalData = "";

		// convert to RGBA
		if (n == 1) {
			for (int i = 0; i < width * height; i++) {
				finalData += data[i * n];
				finalData += data[i * n];
				finalData += data[i * n];
				finalData += '\xff';
			}
		} else if (n == 2) {
			for (int i = 0; i < width * height; i++) {
				finalData += data[i * n];
				finalData += data[i * n];
				finalData += data[i * n];
				finalData += data[i * n + 1];
			}
		} else if (n == 3) {
			for (int i = 0; i < width * height; i++) {
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

		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, finalData.c_str());
		glGenerateMipmap(GL_TEXTURE_2D);
	}

	Texture::~Texture() {
		glDeleteTextures(1, &impl->texId);
	}

	void Texture::activate() {
		glBindTexture(GL_TEXTURE_2D, impl->texId);
	}
}
