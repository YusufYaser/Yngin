#pragma once
#include <memory>

namespace Yngin {
	class Context;

	class Texture {
	public:
		void activate();

	private:
		friend class TexturesManager;
		friend struct std::default_delete<Texture>;

		Texture(Context* ctx, int width, int height, int numCh, const char* data);
		~Texture();

		struct Impl;
		std::unique_ptr<Impl> impl;
	};

	class TexturesManager {
	public:
		uint32_t createTexture(int width, int height, int numCh, const char* data);
		void deleteTexture(uint32_t textureId);

		Texture* getTexture(uint32_t textureId);

	private:
		friend class Context;
		friend struct std::default_delete<TexturesManager>;

		TexturesManager(Context* ctx);
		~TexturesManager();

		struct Impl;
		std::unique_ptr<Impl> impl;
	};
}
