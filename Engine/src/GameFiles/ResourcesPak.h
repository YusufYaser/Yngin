#pragma once
#include <stdint.h>
#include <glm/vec3.hpp>
#include <glm/vec2.hpp>

namespace Yngin {
	enum class TEXTURE_WRAP : uint8_t;
	enum class TEXTURE_FILTER : uint8_t;
	enum class MODEL_FRONT_FACE : uint8_t;

	namespace GameFiles {
		extern const int VERSION;

		namespace ResourcesPak {
			enum TEXTURE_FORMAT : uint8_t {
				PNG,
				PATH,
				RAW
			};

#pragma pack(push, 1)

			struct Header {
				char magic[15];
				uint8_t version;
			};

			static_assert(sizeof(Header) == 16, "Invalid resources.pak header size");

			struct PakModelData {
				uint32_t id;
				MODEL_FRONT_FACE frontFace;
				uint8_t materialsCount;
				uint32_t defaultMaterials[256];
				uint16_t verticesCount;
				uint16_t indicesCount;

				// ModelVertex vertices[verticesCount]
				// uint32_t indices[indicesCount]
			};

			struct ModelVertexData {
				glm::vec3 position;
				glm::vec2 texCoord;
				glm::vec3 normal;
				uint8_t material;
			};

			struct PakMaterialData {
				uint32_t id;
				glm::vec3 ambientColor;
				glm::vec3 diffuseColor;
				glm::vec3 specularColor;
				float specularComponent;
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

			struct TextureRawDataHeader {
				uint32_t width;
				uint32_t height;
				uint8_t numCh;
			};

			struct ScriptData {
				uint32_t id;
				bool enabled;
				uint32_t scene;
				size_t dataSize;

				// unsigned char byteCode[dataSize]
			};

#pragma pack(pop)
		}
	}
}