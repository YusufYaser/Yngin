#include <Yngin/Services/SerializationService.h>
#include <Yngin/Core/Models.h>
#include <Yngin/Core/Materials.h>
#include "../SerializationService_Internal.h"
#include "SerializationStructs.h"
#include <sstream>

using namespace Yngin::Services::Serialization;

namespace Yngin::Services {
	bool SerializationService::serialize(std::ostream& out, ModelsManager* input, bool includeDependencies) {
		input->getContext()->makeCurrent();
		auto models = input->getModels();

		std::stringstream s;

		for (auto& model : models) {
			if (!serialize(s, model, includeDependencies)) return false;
		}

		if (!s.view().empty()) out << s.rdbuf();
		return out.good();
	}

	bool SerializationService::serialize(std::ostream& out, Model* model, bool includeDependencies) {
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

		if (includeDependencies && out.good()) {
			MaterialsManager* materialsManager = model->getContext()->getMaterialsManager();

			for (uint32_t i = 0; i < pakModelData.materialsCount; i++) {
				Material* material = materialsManager->getMaterial(pakModelData.defaultMaterials[i]);

				if (material != nullptr) {
					if (!serialize(s, material)) return false;
				}
			}
		}

		return out.good();
	}

	bool SerializationService::Impl::validateModel(std::istream& in, const Serialization::OperationData& op) {
		SerializedModelData header;

		if (!streamCheck(in, op.headerSize)) return false;
		in.read(reinterpret_cast<char*>(&header), op.headerSize);

		if (header.id == -1) return false;

		switch (header.frontFace) {
		case S_MODEL_FRONT_FACE::NONE:
		case S_MODEL_FRONT_FACE::CCW:
		case S_MODEL_FRONT_FACE::CW:
			break;

		default:
			return false;
		}

		size_t totalDataSize = header.vertexSize * header.verticesCount;
		totalDataSize += header.indexSize * header.indicesCount;

		if (!streamCheck(in, totalDataSize)) return false;
		in.seekg(totalDataSize, std::ios::cur);

		if (!validateOperation(in, Operation::META)) return false;

		return true;
	}
}
