#pragma once
#include <stdint.h>

namespace Yngin {
	enum class TEXTURE_WRAP : uint8_t;
	enum class TEXTURE_FILTER : uint8_t;
	enum class MODEL_FRONT_FACE : uint8_t;

	namespace GameData {
		namespace ResourcesPak {
			enum OP : uint8_t {
				MODEL,
				TEXTURE
			};

			enum TEXTURE_FORMAT : uint8_t {
				PNG,
				PATH
			};

#pragma pack(push, 1)

			struct Header {
				char magic[15];
				uint8_t version;
			};

			static_assert(sizeof(Header) == 16, "Invalid resources.pak header size");

			struct Operation {
				OP op;
			};

			struct PakModelData {
				uint32_t id;
				MODEL_FRONT_FACE frontFace;
				uint16_t verticesCount;
				uint16_t indicesCount;

				// ModelVertex vertices[verticesCount]
				// uint32_t indices[indicesCount]
			};

			struct ModelVertexData {
				float position[3];
				float texCoord[2];
				float normal[3];
			};

			struct PakTextureData {
				uint32_t id;
				TEXTURE_WRAP wrap;
				TEXTURE_FILTER filterMin;
				TEXTURE_FILTER filterMag;
				size_t dataSize;
				TEXTURE_FORMAT dataFormat;

				// unsigned char bytes[dataSize]
			};

#pragma pack(pop)
		}
	}
}