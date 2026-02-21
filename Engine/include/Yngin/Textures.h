#pragma once
#include <Yngin/Yngin.h>
#include <memory>

namespace Yngin {
	class Texture {
	public:
		~Texture();

		void activate();

	private:
		Texture(Context* ctx, int width, int height, int numCh, const char* data);
		friend class TexturesManager;

		struct Impl;
		std::unique_ptr<Impl> impl;
	};

	class TexturesManager {
	public:
		TexturesManager(Context* ctx);
		~TexturesManager();

		uint32_t createTexture(int width, int height, int numCh, const char* data);
		void deleteTexture(uint32_t textureId);

		Texture* getTexture(uint32_t textureId);

	private:
		struct Impl;
		std::unique_ptr<Impl> impl;
	};
}
