#pragma once
#include <stdint.h>
#include <glm/glm.hpp>

namespace Yngin::Services::Serialization {
#pragma pack(push, 1)

	constexpr uint16_t schemaVersion = 0;

	enum class Operation : uint8_t {
		NO_OP = 0,
		META,
		MODEL,
		MATERIAL,
		TEXTURE,
		SCRIPT,
		CAMERA,
		GAMEOBJECT,
		COMPONENT
	};

	struct OperationData {
		// The schema version of the structs used by the SerializationService, including the OperationData struct
		uint16_t schemaVersion;
		Operation op;
		// The size of the initial header
		uint16_t headerSize;
		// The size of all the data related to this operation, including the header and other related operations, excluding the main OperationData struct for this data
		uint64_t dataSize;
	};

	// Meta

	enum class S_META_TYPE : uint8_t {
		UNKNOWN = 0,
		INT32,
		FLOAT,
		STRING,
		POINTER // pointer metas are ignored by the SerializationService but they are still included here
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


	// Camera

	struct SerializedCameraData {
		uint32_t id;
		char slug[33];
		glm::vec3 position;
		glm::vec3 orientation;
		float fov;
		float weight;
	};


	// GameObject and Components

	struct SerializedGameObjectData {
		uint32_t id;
		char slug[33];
		uint32_t parent;
		glm::vec3 position;
		glm::vec3 rotation;
		glm::vec3 scale;
		uint32_t childrenCount;
		uint8_t componentsCount;
	};

	enum S_COMPONENT_TYPE : uint8_t {
		INVALID = 0,
		MESH,
		POINT_LIGHT,
		DIRECTIONAL_LIGHT,
		RIGID_BODY,
		BOX_COLLIDER
	};

	struct GenericComponentHeader {
		S_COMPONENT_TYPE type;
		size_t headerSize;
	};

	struct MeshComponentData {
		uint32_t modelId;
		uint32_t textureId;
		glm::vec3 color;
		uint32_t materials[256];
	};

	struct PointLightData {
		float intensity;
		float distance;
		glm::vec3 color;
	};

	struct DirectionalLightData {
		float intensity;
		glm::vec3 color;
	};

	struct RigidBodyData {
		float mass;
		glm::vec3 velocity;
	};

	struct BoxColliderData {
		glm::vec3 offset;
		glm::vec3 size;
	};

#pragma pack(pop)
}
