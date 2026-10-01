#pragma once
#include <stdint.h>
#include <glm/glm.hpp>

namespace Yngin::Services::Serialization {
#pragma pack(push, 1)

	enum class Operation : uint8_t {
		NO_OP = 0,
		META,
		MODEL,
		MATERIAL,
		TEXTURE,
		SCRIPT
	};

	struct OperationData {
		Operation op;
		uint16_t schemaVersion;
		uint16_t headerSize;
		uint64_t dataSize;
	};

	// Meta

	enum class S_META_TYPE : uint8_t {
		UNKNOWN = 0,
		INT32,
		FLOAT,
		STRING,
		POINTER // pointer metas are not serialized but they are still included here
	};

	struct SerializedMetasHeader {
		size_t metasCount;
		uint16_t unitInfoDataSize;
	};

	struct SerializedMetaInfo {
		S_META_TYPE type;
		uint32_t keySize;
		size_t dataSize;

		// char key[keySize]
		// char data[dataSize]
	};

	// Model

	enum class S_MODEL_FRONT_FACE : uint8_t {
		NONE,
		CCW,
		CW
	};

	struct SerializedModelData {
		uint32_t id;
		char slug[33];
		S_MODEL_FRONT_FACE frontFace;
		uint8_t materialsCount;
		uint32_t defaultMaterials[256];
		uint8_t vertexSize;
		uint8_t indexSize;
		uint32_t verticesCount;
		uint32_t indicesCount;

		// ModelVertexData vertices[verticesCount]
		// ModelIndexData indices[indicesCount]
	};

	struct ModelVertexData {
		glm::vec3 position;
		glm::vec2 texCoord;
		glm::vec3 normal;
		uint8_t material;
	};

	struct ModelIndexData {
		uint32_t index;
	};

	// Material

	struct SerializedMaterialData {
		uint32_t id;
		char slug[32];
		glm::vec3 ambientColor;
		glm::vec3 diffuseColor;
		glm::vec3 specularColor;
		float specularComponent;
	};

	// Texture

	enum S_TEXTURE_FORMAT : uint8_t {
		RAW,
		PNG,
		PATH
	};

	enum class S_TEXTURE_WRAP : uint8_t {
		REPEAT,
		CLAMP
	};

	enum class S_TEXTURE_FILTER : uint8_t {
		NEAREST,
		LINEAR,
		NEAREST_MIPMAP_NEAREST,
		LINEAR_MIPMAP_NEAREST,
		NEAREST_MIPMAP_LINEAR,
		LINEAR_MIPMAP_LINEAR
	};

	struct SerializedTextureData {
		uint32_t id;
		char slug[33];
		S_TEXTURE_WRAP wrap;
		S_TEXTURE_FILTER filterMin;
		S_TEXTURE_FILTER filterMag;
		size_t dataSize;
		uint8_t rawDataHeaderSize;
		S_TEXTURE_FORMAT dataFormat;

		// unsigned char bytes[dataSize]
	};

	struct TextureRawDataHeader {
		uint32_t width;
		uint32_t height;
		uint8_t numCh;
	};

	// Script

	struct SerializedScriptData {
		uint32_t id;
		char slug[33];
		bool enabled;
		uint32_t scene;
		size_t dataSize;

		// unsigned char bytes[dataSize]
	};

#pragma pack(pop)
}
