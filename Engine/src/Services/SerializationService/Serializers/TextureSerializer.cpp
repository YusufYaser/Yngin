#include <Yngin/Services/SerializationService.h>
#include <Yngin/Rendering/Textures.h>
#include "../SerializationService_Internal.h"
#include "SerializationStructs.h"
#include <sstream>
#include <glad/glad.h>
#include <stb/stb_image_write.h>
#include <stb/stb_image.h>

using namespace Yngin::Services::Serialization;

namespace Yngin::Services {
	bool SerializationService::serialize(std::ostream& out, TexturesManager* input, bool compressed) {
		input->getContext()->makeCurrent();
		auto items = input->getTextures();

		std::stringstream s;

		for (auto& item : items) {
			if (!serialize(s, item, compressed)) return false;
		}

		if (!s.view().empty()) out << s.rdbuf();
		return out.good();
	}

	bool SerializationService::serialize(std::ostream& out, Texture* texture, bool compressed) {
		if (impl->shouldSkip(texture->meta)) return true;

		std::stringstream s;

		OperationData op{};
		op.schemaVersion = schemaVersion;
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

			s.write(reinterpret_cast<const char*>(&texData), op.headerSize);
			s.write(context.buf.data(), texData.dataSize);
		} else {
			texData.dataFormat = S_TEXTURE_FORMAT::RAW;
			texData.dataSize = sizeof(TextureRawDataHeader) + data.size();

			TextureRawDataHeader rawDataHeader{};
			rawDataHeader.width = width;
			rawDataHeader.height = height;
			rawDataHeader.numCh = 4;

			s.write(reinterpret_cast<const char*>(&texData), op.headerSize);
			s.write(reinterpret_cast<const char*>(&rawDataHeader), texData.rawDataHeaderSize);
			s.write(reinterpret_cast<const char*>(data.data()), data.size());
		}

		if (!serialize(s, texture->meta)) return false;

		op.dataSize = s.view().size();

		out.write(reinterpret_cast<const char*>(&op), sizeof(OperationData));
		if (!s.view().empty()) out << s.rdbuf();
		return out.good();
	}

	DESERIALIZATION_STATUS SerializationService::Impl::validateTexture(std::istream& in, const Serialization::OperationData& op) {
		SerializedTextureData header{};

		if (!streamCheck(in, op.headerSize, sizeof(header))) return DESERIALIZATION_STATUS::INVALID_DATA;
		in.read(reinterpret_cast<char*>(&header), op.headerSize);

		if (header.id == -1) return DESERIALIZATION_STATUS::INVALID_DATA;

		switch (header.wrap) {
		case S_TEXTURE_WRAP::REPEAT:
		case S_TEXTURE_WRAP::CLAMP:
			break;

		default:
			return DESERIALIZATION_STATUS::INVALID_DATA;
		}

		switch (header.filterMin) {
		case S_TEXTURE_FILTER::NEAREST:
		case S_TEXTURE_FILTER::LINEAR:
		case S_TEXTURE_FILTER::NEAREST_MIPMAP_NEAREST:
		case S_TEXTURE_FILTER::LINEAR_MIPMAP_NEAREST:
		case S_TEXTURE_FILTER::NEAREST_MIPMAP_LINEAR:
		case S_TEXTURE_FILTER::LINEAR_MIPMAP_LINEAR:
			break;

		default:
			return DESERIALIZATION_STATUS::INVALID_DATA;
		}

		switch (header.filterMag) {
		case S_TEXTURE_FILTER::NEAREST:
		case S_TEXTURE_FILTER::LINEAR:
			break;

		default:
			return DESERIALIZATION_STATUS::INVALID_DATA;
		}

		switch (header.dataFormat) {
		case S_TEXTURE_FORMAT::RAW:
			if (header.rawDataHeaderSize > header.dataSize) return DESERIALIZATION_STATUS::INVALID_DATA;

		case S_TEXTURE_FORMAT::PNG:
		case S_TEXTURE_FORMAT::PATH:
			break;

		default:
			return DESERIALIZATION_STATUS::INVALID_DATA;
		}

		if (!streamCheck(in, header.dataSize, -1)) return DESERIALIZATION_STATUS::INVALID_DATA;
		in.seekg(header.dataSize, std::ios::cur);

		DESERIALIZATION_STATUS metaStatus;
		if ((metaStatus = validateOperation(in, Operation::META)) != DESERIALIZATION_STATUS::OK) return metaStatus;

		return DESERIALIZATION_STATUS::OK;
	}

	DESERIALIZATION_STATUS SerializationService::Impl::deserializeTexture(std::istream& in, const Serialization::OperationData& op, InternalDeserializationContext& dsctx) {
		SerializedTextureData header{};

		if (!streamCheck(in, op.headerSize, sizeof(header))) return DESERIALIZATION_STATUS::INVALID_DATA;
		in.read(reinterpret_cast<char*>(&header), op.headerSize);

		if (ctx->getTexturesManager()->getTexture(header.id)) {
			if (!dsctx.user.overrideConflictingId) {
				// Skip the operation data in case we ignore conflicting id errors
				if (!streamCheck(in, op.dataSize - op.headerSize, -1)) return DESERIALIZATION_STATUS::GENERIC_ERROR;
				in.seekg(op.dataSize - op.headerSize, std::ios::cur);
				return DESERIALIZATION_STATUS::CONFLICTING_ID;
			}
		}

		TextureSettings settings{};

		switch (header.wrap) {
		case S_TEXTURE_WRAP::REPEAT:
			settings.wrap = TEXTURE_WRAP::REPEAT;
			break;

		case S_TEXTURE_WRAP::CLAMP:
			settings.wrap = TEXTURE_WRAP::CLAMP;
			break;

		default:
			return DESERIALIZATION_STATUS::INVALID_DATA;
		}

		switch (header.filterMin) {
		case S_TEXTURE_FILTER::NEAREST:
			settings.filterMin = TEXTURE_FILTER::NEAREST;
			break;

		case S_TEXTURE_FILTER::LINEAR:
			settings.filterMin = TEXTURE_FILTER::LINEAR;
			break;

		case S_TEXTURE_FILTER::NEAREST_MIPMAP_NEAREST:
			settings.filterMin = TEXTURE_FILTER::NEAREST_MIPMAP_NEAREST;
			break;

		case S_TEXTURE_FILTER::LINEAR_MIPMAP_NEAREST:
			settings.filterMin = TEXTURE_FILTER::LINEAR_MIPMAP_NEAREST;
			break;

		case S_TEXTURE_FILTER::NEAREST_MIPMAP_LINEAR:
			settings.filterMin = TEXTURE_FILTER::NEAREST_MIPMAP_LINEAR;
			break;

		case S_TEXTURE_FILTER::LINEAR_MIPMAP_LINEAR:
			settings.filterMin = TEXTURE_FILTER::LINEAR_MIPMAP_LINEAR;
			break;

		default:
			return DESERIALIZATION_STATUS::INVALID_DATA;
		}

		switch (header.filterMag) {
		case S_TEXTURE_FILTER::NEAREST:
			settings.filterMag = TEXTURE_FILTER::NEAREST;
			break;

		case S_TEXTURE_FILTER::LINEAR:
			settings.filterMag = TEXTURE_FILTER::LINEAR;
			break;

		default:
			return DESERIALIZATION_STATUS::INVALID_DATA;
		}

		Texture* tex = nullptr;

		if (header.dataFormat == S_TEXTURE_FORMAT::RAW) {
			if (header.rawDataHeaderSize > header.dataSize) return DESERIALIZATION_STATUS::INVALID_DATA;

			TextureRawDataHeader rawHdr{};
			if (!streamCheck(in, header.rawDataHeaderSize, sizeof(rawHdr))) return DESERIALIZATION_STATUS::INVALID_DATA;
			in.read(reinterpret_cast<char*>(&rawHdr), header.rawDataHeaderSize);

			TextureData data{};
			data.width = rawHdr.width;
			data.height = rawHdr.height;
			data.numCh = rawHdr.numCh;

			std::vector<char> bytes(header.dataSize - header.rawDataHeaderSize);
			if (!streamCheck(in, header.dataSize - header.rawDataHeaderSize, bytes.size())) return DESERIALIZATION_STATUS::INVALID_DATA;
			in.read(reinterpret_cast<char*>(bytes.data()), header.dataSize - header.rawDataHeaderSize);

			data.bytes = bytes.data();

			tex = ctx->getTexturesManager()->createTexture(data, settings, header.id, dsctx.user.overrideConflictingId);
		} else if (header.dataFormat == S_TEXTURE_FORMAT::PNG) {
			std::vector<char> pngBytes(header.dataSize);
			if (!streamCheck(in, header.dataSize, pngBytes.size())) return DESERIALIZATION_STATUS::INVALID_DATA;
			in.read(reinterpret_cast<char*>(pngBytes.data()), header.dataSize);

			TextureData data{};
			unsigned char* rawBytes = stbi_load_from_memory((const stbi_uc*)pngBytes.data(), int(pngBytes.size()), &data.width, &data.height, &data.numCh, 0);
			if (!rawBytes) return DESERIALIZATION_STATUS::GENERIC_ERROR;

			data.bytes = (const char*)rawBytes;

			tex = ctx->getTexturesManager()->createTexture(data, settings, header.id, dsctx.user.overrideConflictingId);

			stbi_image_free(rawBytes);

		} else if (header.dataFormat == S_TEXTURE_FORMAT::PATH) {
			if (!dsctx.user.allowLoadingTexturesFromDevice) return DESERIALIZATION_STATUS::PERMISSION_ERROR;

			std::vector<char> bytes(header.dataSize);
			if (!streamCheck(in, header.dataSize, bytes.size())) return DESERIALIZATION_STATUS::INVALID_DATA;
			in.read(reinterpret_cast<char*>(bytes.data()), header.dataSize);

			tex = ctx->getTexturesManager()->createTexture(bytes.data(), settings, header.id, dsctx.user.overrideConflictingId);
		} else {
			return DESERIALIZATION_STATUS::INVALID_DATA;
		}

		if (tex == nullptr) return DESERIALIZATION_STATUS::GENERIC_ERROR;

		dsctx.meta.push(&tex->meta);
		DESERIALIZATION_STATUS metaStatus = deserializeOperation(in, dsctx, Operation::META);
		dsctx.meta.pop();
		if (metaStatus != DESERIALIZATION_STATUS::OK) return metaStatus;

		return DESERIALIZATION_STATUS::OK;
	}
}
