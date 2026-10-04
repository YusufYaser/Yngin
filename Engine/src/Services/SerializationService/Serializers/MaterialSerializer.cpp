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

		out << s.rdbuf();
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
		out << s.rdbuf();
		return out.good();
	}
}
