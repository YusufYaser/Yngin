#pragma once
#include <memory>
#include <glm/vec2.hpp>

namespace Yngin {
	class Context;

	enum class TEXTURE_WRAP : uint8_t {
		REPEAT,
		CLAMP
	};

	enum class TEXTURE_FILTER : uint8_t {
		NEAREST,
		LINEAR,
		NEAREST_MIPMAP_NEAREST,
		LINEAR_MIPMAP_NEAREST,
		NEAREST_MIPMAP_LINEAR,
		LINEAR_MIPMAP_LINEAR
	};

	struct TextureSettings {
		TEXTURE_WRAP wrap = TEXTURE_WRAP::REPEAT;
		TEXTURE_FILTER filterMin = TEXTURE_FILTER::LINEAR_MIPMAP_LINEAR;
		// only LINEAR and NEAREST should be used
		TEXTURE_FILTER filterMag = TEXTURE_FILTER::LINEAR;
	};

	struct TextureData {
		int width;
		int height;
		int numCh;

		const char* bytes;
	};

	class Texture {
	public:
		uint32_t getId() const;
		Context* getContext() const;

		void activate();

		void setData(const TextureData& data, const TextureSettings& settings = {});
		void setData(const char* path, const TextureSettings& settings = {});

		glm::ivec2 getSize() const;

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
		Texture* createTexture();
		Texture* createTexture(const TextureData& data, const TextureSettings& settings = {});
		Texture* createTexture(const char* path, const TextureSettings& settings = {});
		void deleteTexture(uint32_t textureId);

		Texture* getTexture(uint32_t textureId) const;

		Texture* getActive() const;
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
