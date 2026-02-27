#pragma once
#include <memory>

namespace Yngin {
	class Context;

	enum class TEXTURE_WRAP : uint8_t {
		REPEAT,
		CLAMP
	};

	enum class TEXTURE_FILTER : uint8_t {
		LINEAR,
		NEAREST
	};

	struct TextureData {
		int width;
		int height;
		int numCh;
		TEXTURE_WRAP wrap = TEXTURE_WRAP::REPEAT;
		TEXTURE_FILTER filter = TEXTURE_FILTER::LINEAR;

		const char* data;
	};

	class Texture {
	public:
		void activate();

		void setData(TextureData& data);

	private:
		friend class TexturesManager;
		friend struct std::default_delete<Texture>;

		Texture(Context* ctx);
		~Texture();

		struct Impl;
		std::unique_ptr<Impl> impl;
	};

	class TexturesManager {
	public:
		uint32_t createTexture();
		uint32_t createTexture(TextureData& data);
		void deleteTexture(uint32_t textureId);

		Texture* getTexture(uint32_t textureId);

		Texture* getActive();
		void setActive(uint32_t textureId);

	private:
		friend class Context;
		friend struct std::default_delete<TexturesManager>;

		TexturesManager(Context* ctx);
		~TexturesManager();

		struct Impl;
		std::unique_ptr<Impl> impl;
	};
}
