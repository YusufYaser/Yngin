#include <Yngin/Services/SerializationService.h>
#include <Yngin/Core/Scripting.h>
#include "../SerializationService_Internal.h"
#include "SerializationStructs.h"
#include <sstream>
#include <Yngin/Core/Scenes.h>
#include "../../../Core/Scripting/Scripting_Internal.h"

using namespace Yngin::Services::Serialization;

namespace Yngin::Services {
	bool SerializationService::serialize(std::ostream& out, ScriptsManager* input) {
		input->getContext()->makeCurrent();
		auto items = input->getScripts();

		std::stringstream s;

		for (auto& item : items) {
			if (!serialize(s, item)) return false;
		}

		if (!s.view().empty()) out << s.rdbuf();
		return out.good();
	}

	bool SerializationService::serialize(std::ostream& out, Script* script) {
		if (impl->shouldSkip(script->meta)) return true;

		std::stringstream s;

		OperationData op{};
		op.schemaVersion = schemaVersion;
		op.op = Operation::SCRIPT;
		op.headerSize = sizeof(SerializedScriptData);

		SerializedScriptData scriptData{};
		scriptData.id = script->getId();
		if (script->getScene() != nullptr) {
			scriptData.scene = script->getScene()->getId();
		} else {
			scriptData.scene = -1;
		}
		scriptData.enabled = script->isEnabled();
		scriptData.dataSize = script->impl->byteCode.size();

		s.write(reinterpret_cast<const char*>(&scriptData), op.headerSize);

		s.write(script->impl->byteCode.data(), scriptData.dataSize);

		if (!serialize(s, script->meta)) return false;

		op.dataSize = s.view().size();

		out.write(reinterpret_cast<const char*>(&op), sizeof(OperationData));
		if (!s.view().empty()) out << s.rdbuf();
		return out.good();
	}

	DESERIALIZATION_STATUS SerializationService::Impl::validateScript(std::istream& in, const Serialization::OperationData& op) {
		SerializedScriptData header;

		if (!streamCheck(in, op.headerSize, sizeof(SerializedScriptData))) return DESERIALIZATION_STATUS::INVALID_DATA;
		in.read(reinterpret_cast<char*>(&header), op.headerSize);

		if (header.id == -1) return DESERIALIZATION_STATUS::INVALID_DATA;

		if (!streamCheck(in, header.dataSize, -1)) return DESERIALIZATION_STATUS::INVALID_DATA;
		in.seekg(header.dataSize, std::ios::cur);

		DESERIALIZATION_STATUS metaStatus;
		if ((metaStatus = validateOperation(in, Operation::META)) != DESERIALIZATION_STATUS::OK) return metaStatus;

		return DESERIALIZATION_STATUS::OK;
	}
}
