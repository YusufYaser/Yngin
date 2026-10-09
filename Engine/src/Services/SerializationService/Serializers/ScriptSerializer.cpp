#include <Yngin/Services/SerializationService.h>
#include <Yngin/Core/Scripting.h>
#include "../SerializationService_Internal.h"
#include "SerializationStructs.h"
#include <sstream>
#include <Yngin/Core/Scenes.h>
#include "../../../Core/Scripting/Scripting_Internal.h"

#define LOGGER_NAME SerializationService
#include "../../../Internal/Logger.h"

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
		if (impl->shouldSkip(script->meta)) {
			TRACE("Skipping script with id %u", script->getId());
			return true;
		}

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
		SerializedScriptData header{};

		if (!streamCheck(in, op.headerSize, sizeof(header))) return DESERIALIZATION_STATUS::INVALID_DATA;
		in.read(reinterpret_cast<char*>(&header), op.headerSize);

		if (header.id == -1) return DESERIALIZATION_STATUS::INVALID_DATA;

		if (!streamCheck(in, header.dataSize, -1)) return DESERIALIZATION_STATUS::INVALID_DATA;
		in.seekg(header.dataSize, std::ios::cur);

		DESERIALIZATION_STATUS metaStatus;
		if ((metaStatus = validateOperation(in, Operation::META)) != DESERIALIZATION_STATUS::OK) return metaStatus;

		return DESERIALIZATION_STATUS::OK;
	}

	DESERIALIZATION_STATUS SerializationService::Impl::deserializeScript(std::istream& in, const Serialization::OperationData& op, InternalDeserializationContext& dsctx) {
		SerializedScriptData header{};

		if (!streamCheck(in, op.headerSize, sizeof(header))) return DESERIALIZATION_STATUS::INVALID_DATA;
		in.read(reinterpret_cast<char*>(&header), op.headerSize);

		if (ctx->getScriptsManager()->getScript(header.id)) {
			if (!dsctx.user.overrideConflictingId) {
				TRACE("Skipping conflicting script id: %i", header.id);
				// Skip the operation data in case we ignore conflicting id errors
				if (!streamCheck(in, op.dataSize - op.headerSize, -1)) return DESERIALIZATION_STATUS::GENERIC_ERROR;
				in.seekg(op.dataSize - op.headerSize, std::ios::cur);
				return DESERIALIZATION_STATUS::CONFLICTING_ID;
			} else {
				TRACE("Overriding conflicting script id: %i", header.id);
			}
		}

		Scene* scriptScene = nullptr;
		if (header.scene != -1) {
			scriptScene = ctx->getScenesManager()->getScene(header.scene);
			if (scriptScene == nullptr) return DESERIALIZATION_STATUS::INVALID_DATA;
		}

		if (!streamCheck(in, header.dataSize, -1)) return DESERIALIZATION_STATUS::INVALID_DATA;
		std::vector<char> data(header.dataSize);
		in.read(reinterpret_cast<char*>(data.data()), header.dataSize);

		Script* script = ctx->getScriptsManager()->createScript(scriptScene, "", header.id, dsctx.user.overrideConflictingId);
		if (script == nullptr) return DESERIALIZATION_STATUS::GENERIC_ERROR;

		script->impl->enabled = header.enabled;

		sol::load_result chunk = ctx->getScriptsManager()->impl->lua.load(std::string_view(data.data(), data.size()));

		if (!chunk.valid()) {
			sol::error error = chunk;

			printf("[Yngin] [Script #%i] Error while loading script from resources: %s\n", header.id, error.what());
		} else {
			script->impl->byteCode = data;

			lua_State* L = ctx->getScriptsManager()->impl->lua.lua_state();

			sol::protected_function func = chunk;

			func.push(L);
			script->impl->env.push(L);
			lua_setupvalue(L, -2, 1);
			lua_pop(L, 1);

			sol::protected_function_result res = func();

			if (!res.valid()) {
				sol::error error = res;

				printf("[Yngin] [Script #%i] Error while loading script from resources:: %s\n", header.id, error.what());
			}
		}

		dsctx.meta.push(&script->meta);
		DESERIALIZATION_STATUS metaStatus = deserializeOperation(in, dsctx, Operation::META);
		dsctx.meta.pop();
		if (metaStatus != DESERIALIZATION_STATUS::OK) return metaStatus;

		return DESERIALIZATION_STATUS::OK;
	}
}
