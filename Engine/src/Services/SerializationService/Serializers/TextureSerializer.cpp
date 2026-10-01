#include <Yngin/Services/SerializationService.h>
#include <Yngin/Rendering/Textures.h>
#include "../SerializationService_Internal.h"
#include "SerializationStructs.h"
#include <sstream>
#include <glad/glad.h>
#include <stb/stb_image_write.h>

using namespace Yngin::Services::Serialization;

namespace Yngin::Services {
	bool SerializationService::serialize(std::ostream& out, TexturesManager* input, bool compressed) {
		input->getContext()->makeCurrent();
		auto items = input->getTextures();

		std::stringstream s;

		for (auto& item : items) {
			if (!serialize(s, item, compressed)) return false;
		}

		out << s.rdbuf();
		return out.good();
	}

	bool SerializationService::serialize(std::ostream& out, Texture* texture, bool compressed) {
		if (impl->shouldSkip(texture->meta)) return true;

		std::stringstream s;

		OperationData op{};
		op.op = Operation::TEXTURE;
		op.headerSize = sizeof(SerializedTextureData);

		const TextureSettings& settings = texture->getTextureSettings();

		SerializedTextureData texData{};
		texData.id = texture->getId();
		texData.rawDataHeaderSize = sizeof(TextureRawDataHeader);

		switch (settings.wrap) {
		case TEXTURE_WRAP::REPEAT:
			texData.wrap = S_TEXTURE_WRAP::REPEAT;
			break;
		case TEXTURE_WRAP::CLAMP:
			texData.wrap = S_TEXTURE_WRAP::CLAMP;
			break;
		default:
			return false;
		}
		switch (settings.filterMin) {
		case TEXTURE_FILTER::NEAREST:
			texData.filterMin = S_TEXTURE_FILTER::NEAREST;
			break;
		case TEXTURE_FILTER::LINEAR:
			texData.filterMin = S_TEXTURE_FILTER::LINEAR;
			break;
		case TEXTURE_FILTER::NEAREST_MIPMAP_NEAREST:
			texData.filterMin = S_TEXTURE_FILTER::NEAREST_MIPMAP_NEAREST;
			break;
		case TEXTURE_FILTER::LINEAR_MIPMAP_NEAREST:
			texData.filterMin = S_TEXTURE_FILTER::LINEAR_MIPMAP_NEAREST;
			break;
		case TEXTURE_FILTER::NEAREST_MIPMAP_LINEAR:
			texData.filterMin = S_TEXTURE_FILTER::NEAREST_MIPMAP_LINEAR;
			break;
		case TEXTURE_FILTER::LINEAR_MIPMAP_LINEAR:
			texData.filterMin = S_TEXTURE_FILTER::LINEAR_MIPMAP_LINEAR;
			break;
		default:
			return false;
		}
		switch (settings.filterMag) {
		case TEXTURE_FILTER::NEAREST:
			texData.filterMag = S_TEXTURE_FILTER::NEAREST;
			break;
		case TEXTURE_FILTER::LINEAR:
			texData.filterMag = S_TEXTURE_FILTER::LINEAR;
			break;
		default:
			return false;
		}

		texData.dataSize = 0;

		Texture* activatedTexture = texture->getContext()->getTexturesManager()->getActive();

		texture->activate();

		int width, height;

		glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, &width);
		glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_HEIGHT, &height);

		std::vector<unsigned char> data(width * height * 4);

		glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_UNSIGNED_BYTE, data.data());

		if (activatedTexture) activatedTexture->activate();
		else glBindTexture(GL_TEXTURE_2D, 0);

		// TODO: add option to serialize as PNG


		if (compressed && width * height > 36) {
			struct WriteContext {
				size_t offset = 0;
				size_t sizeCache = 0;
				std::vector<char> buf;
			} context;

			context.sizeCache = width * height * 4 + 18;
			context.buf.resize(context.sizeCache);

			stbi_write_png_to_func([](void* context, void* data, int size) {
				auto ctx = (WriteContext*)context;

				if (ctx->offset + size > ctx->sizeCache) {
					ctx->sizeCache = ctx->offset + size;
					ctx->buf.resize(ctx->sizeCache);
				}

				memcpy_s(ctx->buf.data() + ctx->offset, size, data, size);

				ctx->offset += size;
				}, &context, width, height, 4, data.data(), 0);


			texData.dataFormat = S_TEXTURE_FORMAT::PNG;
			texData.dataSize = context.offset;

			s.write(reinterpret_cast<const char*>(&texData), op.dataSize);
			s.write(context.buf.data(), texData.dataSize);
		} else {
			texData.dataFormat = S_TEXTURE_FORMAT::RAW;
			texData.dataSize = sizeof(TextureRawDataHeader) + data.size();

			TextureRawDataHeader rawDataHeader{};
			rawDataHeader.width = width;
			rawDataHeader.height = height;
			rawDataHeader.numCh = 4;

			s.write(reinterpret_cast<const char*>(&texData), op.dataSize);
			s.write(reinterpret_cast<const char*>(&rawDataHeader), texData.rawDataHeaderSize);
			s.write(reinterpret_cast<const char*>(data.data()), data.size());
		}

		if (!serialize(s, texture->meta)) return false;

		op.dataSize = s.view().size();

		out.write(reinterpret_cast<const char*>(&op), sizeof(OperationData));
		out << s.rdbuf();
		return out.good();
	}
}
