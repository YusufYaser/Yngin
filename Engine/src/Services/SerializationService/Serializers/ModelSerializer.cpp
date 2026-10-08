#include <Yngin/Services/SerializationService.h>
#include <Yngin/Core/Models.h>
#include <Yngin/Core/Materials.h>
#include "../SerializationService_Internal.h"
#include "SerializationStructs.h"
#include <sstream>

using namespace Yngin::Services::Serialization;

namespace Yngin::Services {
	bool SerializationService::serialize(std::ostream& out, ModelsManager* input) {
		input->getContext()->makeCurrent();
		auto models = input->getModels();

		std::stringstream s;

		for (auto& model : models) {
			if (!serialize(s, model)) return false;
		}

		if (!s.view().empty()) out << s.rdbuf();
		return out.good();
	}

	bool SerializationService::serialize(std::ostream& out, Model* model) {
		if (impl->shouldSkip(model->meta)) return true;

		std::stringstream s;

		OperationData op{};
		op.schemaVersion = schemaVersion;
		op.op = Operation::MODEL;
		op.headerSize = sizeof(SerializedModelData);

		const ModelData& data = model->getModelData();

		SerializedModelData pakModelData{};
		pakModelData.id = model->getId();

		switch (data.frontFace) {
		case MODEL_FRONT_FACE::NONE:
			pakModelData.frontFace = S_MODEL_FRONT_FACE::NONE;
			break;

		case MODEL_FRONT_FACE::CCW:
			pakModelData.frontFace = S_MODEL_FRONT_FACE::CCW;
			break;

		case MODEL_FRONT_FACE::CW:
			pakModelData.frontFace = S_MODEL_FRONT_FACE::CW;
			break;

		default:
			return false;
		}

		pakModelData.materialsCount = data.materialsCount;
		for (uint32_t i = 0; i < data.materialsCount; i++) {
			pakModelData.defaultMaterials[i] = data.defaultMaterials[i];
		}

		pakModelData.vertexSize = sizeof(ModelVertexData);
		pakModelData.indexSize = sizeof(ModelIndexData);

		pakModelData.verticesCount = uint32_t(data.vertices.size());
		pakModelData.indicesCount = uint32_t(data.indices.size());

		s.write(reinterpret_cast<const char*>(&pakModelData), op.headerSize);

		for (uint32_t i = 0; i < pakModelData.verticesCount; i++) {
			Vertex vertex = data.vertices[i];
			ModelVertexData v{};
			v.position = vertex.pos;
			v.texCoord = vertex.texCoord;
			v.normal = vertex.normal;
			v.material = vertex.matId;

			s.write(reinterpret_cast<const char*>(&v), pakModelData.vertexSize);
		}

		for (uint32_t i = 0; i < pakModelData.indicesCount; i++) {
			ModelIndexData index = { data.indices[i] };

			s.write(reinterpret_cast<const char*>(&index), pakModelData.indexSize);
		}

		if (!serialize(s, model->meta)) return false;

		op.dataSize = s.view().size();

		out.write(reinterpret_cast<const char*>(&op), sizeof(OperationData));
		if (!s.view().empty()) out << s.rdbuf();

		return out.good();
	}

	DESERIALIZATION_STATUS SerializationService::Impl::validateModel(std::istream& in, const Serialization::OperationData& op) {
		SerializedModelData header{};

		if (!streamCheck(in, op.headerSize, sizeof(SerializedModelData))) return DESERIALIZATION_STATUS::INVALID_DATA;
		in.read(reinterpret_cast<char*>(&header), op.headerSize);

		if (header.id == -1) return DESERIALIZATION_STATUS::INVALID_DATA;

		switch (header.frontFace) {
		case S_MODEL_FRONT_FACE::NONE:
		case S_MODEL_FRONT_FACE::CCW:
		case S_MODEL_FRONT_FACE::CW:
			break;

		default:
			return DESERIALIZATION_STATUS::INVALID_DATA;
		}

		size_t totalDataSize = header.vertexSize * header.verticesCount;
		totalDataSize += header.indexSize * header.indicesCount;

		if (!streamCheck(in, totalDataSize, -1)) return DESERIALIZATION_STATUS::INVALID_DATA;
		in.seekg(totalDataSize, std::ios::cur);

		DESERIALIZATION_STATUS metaStatus;
		if ((metaStatus = validateOperation(in, Operation::META)) != DESERIALIZATION_STATUS::OK) return metaStatus;

		return DESERIALIZATION_STATUS::OK;
	}

	DESERIALIZATION_STATUS SerializationService::Impl::deserializeModel(std::istream& in, const Serialization::OperationData& op, InternalDeserializationContext& dsctx) {
		SerializedModelData header{};

		if (!streamCheck(in, op.headerSize, sizeof(SerializedModelData))) return DESERIALIZATION_STATUS::INVALID_DATA;
		in.read(reinterpret_cast<char*>(&header), op.headerSize);

		if (ctx->getModelsManager()->getModel(header.id)) {
			if (!dsctx.user.overrideConflictingId) {
				// Skip the operation data in case we ignore conflicting id errors
				if (streamCheck(in, op.dataSize - op.headerSize, -1)) return DESERIALIZATION_STATUS::GENERIC_ERROR;
				in.seekg(op.dataSize - op.headerSize, std::ios::cur);
				return DESERIALIZATION_STATUS::CONFLICTING_ID;
			}
		}

		ModelData data{};

		switch (header.frontFace) {
		case S_MODEL_FRONT_FACE::NONE:
			data.frontFace = MODEL_FRONT_FACE::NONE;
			break;

		case S_MODEL_FRONT_FACE::CCW:
			data.frontFace = MODEL_FRONT_FACE::CCW;
			break;

		case S_MODEL_FRONT_FACE::CW:
			data.frontFace = MODEL_FRONT_FACE::CW;
			break;

		default:
			return DESERIALIZATION_STATUS::INVALID_DATA;
		}

		for (uint32_t i = 0; i < header.verticesCount; i++) {
			ModelVertexData vertexData{};
			if (!streamCheck(in, header.vertexSize, sizeof(vertexData))) return DESERIALIZATION_STATUS::INVALID_DATA;
			in.read(reinterpret_cast<char*>(&vertexData), header.vertexSize);

			Vertex v{};
			v.pos = vertexData.position;
			v.normal = vertexData.normal;
			v.texCoord = vertexData.texCoord;
			v.matId = vertexData.material;

			data.vertices.push_back(v);
		}

		for (uint32_t i = 0; i < header.indicesCount; i++) {
			ModelIndexData indexData{};
			if (!streamCheck(in, header.indexSize, sizeof(indexData))) return DESERIALIZATION_STATUS::INVALID_DATA;
			in.read(reinterpret_cast<char*>(&indexData), header.indexSize);

			data.indices.push_back(indexData.index);
		}

		data.materialsCount = header.materialsCount;
		for (uint32_t i = 0; i < data.materialsCount; i++) {
			data.defaultMaterials[i] = header.defaultMaterials[i];
		}

		Model* model = ctx->getModelsManager()->createModel(data, header.id, dsctx.user.overrideConflictingId);
		if (model == nullptr) return DESERIALIZATION_STATUS::GENERIC_ERROR;

		dsctx.meta.push(&model->meta);
		DESERIALIZATION_STATUS metaStatus = deserializeOperation(in, dsctx, Operation::META);
		dsctx.meta.pop();
		if (metaStatus != DESERIALIZATION_STATUS::OK) return metaStatus;

		return DESERIALIZATION_STATUS::OK;
	}
}
