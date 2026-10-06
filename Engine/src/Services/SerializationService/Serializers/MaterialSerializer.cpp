#include <Yngin/Services/SerializationService.h>
#include <Yngin/Core/Materials.h>
#include "../SerializationService_Internal.h"
#include "SerializationStructs.h"
#include <sstream>

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
		if (impl->shouldSkip(material->meta)) return true;

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
		SerializedMaterialData header;

		if (!streamCheck(in, op.headerSize)) return DESERIALIZATION_STATUS::INVALID_DATA;
		in.read(reinterpret_cast<char*>(&header), op.headerSize);

		if (header.id == -1) return DESERIALIZATION_STATUS::INVALID_DATA;

		DESERIALIZATION_STATUS metaStatus;
		if ((metaStatus = validateOperation(in, Operation::META)) != DESERIALIZATION_STATUS::OK) return metaStatus;

		return DESERIALIZATION_STATUS::OK;
	}
}
