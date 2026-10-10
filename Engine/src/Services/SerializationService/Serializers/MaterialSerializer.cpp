#include <Yngin/Services/SerializationService.h>
#include <Yngin/Core/Materials.h>
#include "../SerializationService_Internal.h"
#include "SerializationStructs.h"
#include <sstream>

#define LOGGER_NAME SerializationService
#include "../../../Internal/Logger.h"

using namespace Yngin::Services::Serialization;

namespace Yngin::Services {
	bool SerializationService::serialize(std::ostream& out, MaterialsManager* input) {
		input->getContext()->makeCurrent();
		auto items = input->getMaterials();

		std::stringstream s;

		for (auto& item : items) {
			if (!serialize(s, item)) return false;
		}

		if (!s.view().empty()) out << s.rdbuf();
		return out.good();
	}

	bool SerializationService::serialize(std::ostream& out, Material* material) {
		if (impl->shouldSkip(material->meta)) {
			TRACE("Skipping material with id %u", material->getId());
			return true;
		}

		std::stringstream s;

		OperationData op{};
		op.schemaVersion = schemaVersion;
		op.op = Operation::MATERIAL;
		op.headerSize = sizeof(SerializedMaterialData);

		SerializedMaterialData data{};
		data.id = material->getId();

		data.ambientColor = material->getAmbientColor();
		data.diffuseColor = material->getDiffuseColor();
		data.specularColor = material->getSpecularColor();
		data.specularComponent = material->getSpecularComponent();

		s.write(reinterpret_cast<const char*>(&data), op.headerSize);

		if (!serialize(s, material->meta)) return false;

		op.dataSize = s.view().size();

		out.write(reinterpret_cast<const char*>(&op), sizeof(OperationData));
		if (!s.view().empty()) out << s.rdbuf();
		return out.good();
	}

	DESERIALIZATION_STATUS SerializationService::Impl::validateMaterial(std::istream& in, const Serialization::OperationData& op) {
		SerializedMaterialData header{};

		if (!streamCheck(in, op.headerSize, sizeof(SerializedMaterialData))) return DESERIALIZATION_STATUS::INVALID_DATA;
		in.read(reinterpret_cast<char*>(&header), op.headerSize);

		if (header.id == -1) return DESERIALIZATION_STATUS::INVALID_DATA;

		DESERIALIZATION_STATUS metaStatus;
		if ((metaStatus = validateOperation(in, Operation::META)) != DESERIALIZATION_STATUS::OK) return metaStatus;

		return DESERIALIZATION_STATUS::OK;
	}

	DESERIALIZATION_STATUS SerializationService::Impl::deserializeMaterial(std::istream& in, const Serialization::OperationData& op, InternalDeserializationContext& dsctx) {
		SerializedMaterialData header{};

		if (!streamCheck(in, op.headerSize, sizeof(SerializedMaterialData))) return DESERIALIZATION_STATUS::INVALID_DATA;
		in.read(reinterpret_cast<char*>(&header), op.headerSize);

		Material* material = nullptr;

		if (material = ctx->getMaterialsManager()->getMaterial(header.id)) {
			if (dsctx.user.overrideConflictingId) {
				TRACE("Overriding conflicting material id: %i", header.id);
				material = nullptr;
			} else if (dsctx.user.useOriginalConflictingObject) {
				TRACE("Using original conflicting material id: %i", header.id);
			} else {
				TRACE("Skipping conflicting material id: %i", header.id);
				// Skip the operation data in case we ignore conflicting id errors
				if (!streamCheck(in, op.dataSize - op.headerSize, -1)) return DESERIALIZATION_STATUS::GENERIC_ERROR;
				in.seekg(op.dataSize - op.headerSize, std::ios::cur);
				return DESERIALIZATION_STATUS::CONFLICTING_ID;
			}
		}

		if (material == nullptr) material = ctx->getMaterialsManager()->createMaterial(header.id, dsctx.user.overrideConflictingId);
		if (material == nullptr) return DESERIALIZATION_STATUS::GENERIC_ERROR;
		material->setAmbientColor(header.ambientColor);
		material->setDiffuseColor(header.diffuseColor);
		material->setSpecularColor(header.specularColor);
		material->setSpecularComponent(header.specularComponent);

		dsctx.meta.push(&material->meta);
		DESERIALIZATION_STATUS metaStatus = deserializeOperation(in, dsctx, Operation::META);
		dsctx.meta.pop();
		if (metaStatus != DESERIALIZATION_STATUS::OK) return metaStatus;

		return DESERIALIZATION_STATUS::OK;
	}
}
