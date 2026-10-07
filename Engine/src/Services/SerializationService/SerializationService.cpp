#include <Yngin/Services/SerializationService.h>
#include "SerializationService_Internal.h"
#include "Serializers/SerializationStructs.h"
#include <iostream>

using namespace Yngin::Services::Serialization;

namespace Yngin::Services {
	SerializationService::SerializationService(Context* ctx) : Service(ctx) {
		impl = std::make_unique<Impl>();
		impl->ctx = ctx;

		pushSkipMetaKeys({ "#NoExport" });
		pushIgnoredMetaPrefixes({ "#" });
	}

	SerializationService::~SerializationService() = default;

	void SerializationService::pushSkipMetaKeys(const std::vector<std::string>& values) {
		impl->skipMetaKeys.push_back(values);
		impl->currentSkipMetaKeys = impl->skipMetaKeys.back();
	}

	std::vector<std::string> SerializationService::popSkipMetaKeys() {
		if (impl->skipMetaKeys.empty()) {
			return {};
		}

		auto val = impl->skipMetaKeys.back();
		impl->skipMetaKeys.pop_back();

		if (!impl->skipMetaKeys.empty()) {
			impl->currentSkipMetaKeys = impl->skipMetaKeys.back();
		} else {
			impl->currentSkipMetaKeys.clear();
		}

		return val;
	}

	void SerializationService::pushIgnoredMetaPrefixes(const std::vector<std::string>& values) {
		impl->ignoredMetaPrefixes.push_back(values);
		impl->currentIgnoredMetaPrefixes = impl->ignoredMetaPrefixes.back();
	}

	std::vector<std::string> SerializationService::popIgnoredMetaPrefixes() {
		if (impl->ignoredMetaPrefixes.empty()) {
			return {};
		}

		auto val = impl->ignoredMetaPrefixes.back();
		impl->ignoredMetaPrefixes.pop_back();

		if (!impl->ignoredMetaPrefixes.empty()) {
			impl->currentIgnoredMetaPrefixes = impl->ignoredMetaPrefixes.back();
		} else {
			impl->currentIgnoredMetaPrefixes.clear();
		}

		return val;
	}

	bool SerializationService::Impl::shouldSkip(const Meta& meta) {
		if (meta.getMetasCount() == 0) {
			return false;
		}

		static const MetaValue empty{};

		for (auto& key : currentSkipMetaKeys) {
			if (meta.getMeta(key, empty) != empty) {
				return true;
			}
		}
		return false;
	}

	DESERIALIZATION_STATUS SerializationService::validate(std::istream& in) {
		std::streampos originalPos = in.tellg();
		if (originalPos == std::streampos(-1)) {
			return DESERIALIZATION_STATUS::STREAM_ERROR;
		}

		DESERIALIZATION_STATUS status = DESERIALIZATION_STATUS::OK;
		while (status == DESERIALIZATION_STATUS::OK) {
			std::streampos pos = in.tellg();
			in.seekg(0, std::ios::end);
			bool atEnd = pos == in.tellg();
			in.seekg(pos);
			if (atEnd) break;

			status = impl->validateOperation(in);
		}

		in.clear();
		in.seekg(originalPos);

		return status;
	}

	bool SerializationService::Impl::streamCheck(std::istream& in, size_t size, size_t structSize) {
		if (structSize != -1 && size > structSize) return false;
		if (!in.good()) return false;

		std::streampos cur = in.tellg();
		in.seekg(0, std::ios::end);

		std::streampos end = in.tellg();
		in.seekg(cur);

		if (end - cur < size) return false;

		return true;
	}

	DESERIALIZATION_STATUS SerializationService::Impl::validateOperation(std::istream& in, const Operation& expectedOp) {
		OperationData op;

		if (!streamCheck(in, sizeof(uint16_t), sizeof(uint16_t))) return DESERIALIZATION_STATUS::INVALID_DATA;
		in.read(reinterpret_cast<char*>(&op.schemaVersion), sizeof(uint16_t));

		if (op.schemaVersion > Serialization::schemaVersion) return DESERIALIZATION_STATUS::UNSUPPORTED_SCHEMA_VERSION;

		in.seekg(-2, std::ios::cur);

		if (!streamCheck(in, sizeof(OperationData), sizeof(OperationData))) return DESERIALIZATION_STATUS::INVALID_DATA;
		in.read(reinterpret_cast<char*>(&op), sizeof(OperationData));

		if (op.headerSize > op.dataSize) return DESERIALIZATION_STATUS::INVALID_DATA;

		if (!streamCheck(in, op.dataSize, -1)) return DESERIALIZATION_STATUS::INVALID_DATA;

		if (expectedOp != Operation::NO_OP && op.op != expectedOp) return DESERIALIZATION_STATUS::INVALID_DATA;

		switch (op.op) {
		case Operation::NO_OP:
		{
			in.seekg(op.dataSize, std::ios::cur);
			return DESERIALIZATION_STATUS::OK;
		}

		case Operation::META:
			return validateMeta(in, op);

		case Operation::MODEL:
			return validateModel(in, op);

		case Operation::MATERIAL:
			return validateMaterial(in, op);

		case Operation::TEXTURE:
			return validateTexture(in, op);

		case Operation::SCRIPT:
			return validateScript(in, op);

		case Operation::CAMERA:
			return validateCamera(in, op);

		case Operation::GAMEOBJECT:
			return validateGameObject(in, op);

		case Operation::COMPONENT:
			return validateComponent(in, op);

		case Operation::UI_ELEMENT:
			return validateUIElement(in, op);

		default:
			return DESERIALIZATION_STATUS::INVALID_DATA;
		}

		return DESERIALIZATION_STATUS::INVALID_DATA;
	}

	DESERIALIZATION_STATUS SerializationService::load(std::istream& in, const DeserializationContext& deserializationContext) {
		DESERIALIZATION_STATUS validationStatus = validate(in);
		if (validationStatus != DESERIALIZATION_STATUS::OK) return validationStatus;

		InternalDeserializationContext internalDsctx{ deserializationContext };

		internalDsctx.meta.push(deserializationContext.targetMeta);
		internalDsctx.scene.push(deserializationContext.targetScene);
		internalDsctx.gameObject.push(deserializationContext.targetGameObject);
		internalDsctx.UIManager.push(deserializationContext.targetUIManager);

		DESERIALIZATION_STATUS status = DESERIALIZATION_STATUS::OK;
		while (status == DESERIALIZATION_STATUS::OK) {
			std::streampos pos = in.tellg();
			in.seekg(0, std::ios::end);
			bool atEnd = pos == in.tellg();
			in.seekg(pos);
			if (atEnd) break;

			status = impl->deserializeOperation(in, internalDsctx);
			if ((deserializationContext.ignoreErrorMissingContext && status == DESERIALIZATION_STATUS::MISSING_CONTEXT)
				|| (deserializationContext.ignoreErrorConflictingId && status == DESERIALIZATION_STATUS::CONFLICTING_ID)) {
				status = DESERIALIZATION_STATUS::OK;
			}
		}

		return status;
	}

	DESERIALIZATION_STATUS SerializationService::Impl::deserializeOperation(std::istream& in, InternalDeserializationContext& dsctx, const Serialization::Operation& expectedOp) {
		OperationData op;

		in.read(reinterpret_cast<char*>(&op.schemaVersion), sizeof(uint16_t));

		if (op.schemaVersion > Serialization::schemaVersion) return DESERIALIZATION_STATUS::UNSUPPORTED_SCHEMA_VERSION;

		in.seekg(-2, std::ios::cur);

		in.read(reinterpret_cast<char*>(&op), sizeof(OperationData));

		if (op.headerSize > op.dataSize) return DESERIALIZATION_STATUS::INVALID_DATA;

		if (!streamCheck(in, op.dataSize, -1)) return DESERIALIZATION_STATUS::INVALID_DATA;

		if (expectedOp != Operation::NO_OP && op.op != expectedOp) return DESERIALIZATION_STATUS::INVALID_DATA;

		switch (op.op) {
		case Operation::NO_OP:
		{
			in.seekg(op.dataSize, std::ios::cur);
			return DESERIALIZATION_STATUS::OK;
		}

		case Operation::META:
			return deserializeMeta(in, op, dsctx);

		case Operation::MODEL:
			return deserializeModel(in, op, dsctx);

		case Operation::MATERIAL:
			return deserializeMaterial(in, op, dsctx);

		case Operation::TEXTURE:
			return deserializeTexture(in, op, dsctx);

		case Operation::SCRIPT:
			return deserializeScript(in, op, dsctx);

		case Operation::CAMERA:
			return deserializeCamera(in, op, dsctx);

		case Operation::GAMEOBJECT:
			return deserializeGameObject(in, op, dsctx);

		case Operation::COMPONENT:
			return deserializeComponent(in, op, dsctx);

		case Operation::UI_ELEMENT:
			return deserializeUIElement(in, op, dsctx);

		default:
			return DESERIALIZATION_STATUS::INVALID_DATA;
		}

		return DESERIALIZATION_STATUS::INVALID_DATA;
	}
}
