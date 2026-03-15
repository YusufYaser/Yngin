#pragma once
#include <stdint.h>

namespace Yngin {
	namespace GameData {
		namespace ResourcesPak {
			enum OP : uint8_t {
				MODEL
			};

			enum PAK_MODEL_FRONT_FACE : uint8_t {
				NONE,
				CCW,
				CW
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
				PAK_MODEL_FRONT_FACE frontFace;
				uint8_t verticesCount;
				uint8_t indicesCount;
				uint8_t unused;

				// ModelVertex vertices[verticesCount]
				// uint32_t indices[indicesCount]
			};

			struct ModelVertexData {
				float position[3];
				float texCoord[2];
				float normal[3];
			};

#pragma pack(pop)
		}
	}
}