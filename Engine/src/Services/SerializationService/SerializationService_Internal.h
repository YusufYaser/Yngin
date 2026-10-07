#pragma once
#include <Yngin/Services/SerializationService.h>
#include <Yngin/Utils/Meta.h>
#include "Serializers/SerializationStructs.h"

namespace Yngin::Services {
	struct InternalDeserializationContext {
		DeserializationContext user;
		Meta* meta;
	};

	struct SerializationService::Impl {
		Context* ctx;

		std::vector<std::vector<std::string>> skipMetaKeys;
		std::vector<std::string> currentSkipMetaKeys;
		std::vector<std::vector<std::string>> ignoredMetaPrefixes;
		std::vector<std::string> currentIgnoredMetaPrefixes;

		bool shouldSkip(const Meta& meta);


		// Validators
		bool streamCheck(std::istream& in, size_t size, size_t structSize);
		DESERIALIZATION_STATUS validateOperation(std::istream& in, const Serialization::Operation& expectedOp = Serialization::Operation::NO_OP);
		DESERIALIZATION_STATUS validateMeta(std::istream& in, const Serialization::OperationData& op);
		DESERIALIZATION_STATUS validateModel(std::istream& in, const Serialization::OperationData& op);
		DESERIALIZATION_STATUS validateMaterial(std::istream& in, const Serialization::OperationData& op);
		DESERIALIZATION_STATUS validateTexture(std::istream& in, const Serialization::OperationData& op);
		DESERIALIZATION_STATUS validateScript(std::istream& in, const Serialization::OperationData& op);
		DESERIALIZATION_STATUS validateCamera(std::istream& in, const Serialization::OperationData& op);
		DESERIALIZATION_STATUS validateGameObject(std::istream& in, const Serialization::OperationData& op);
		DESERIALIZATION_STATUS validateComponent(std::istream& in, const Serialization::OperationData& op);
		DESERIALIZATION_STATUS validateUIElement(std::istream& in, const Serialization::OperationData& op);

		// Validators
		DESERIALIZATION_STATUS deserializeOperation(std::istream& in, InternalDeserializationContext& dsctx, const Serialization::Operation& expectedOp = Serialization::Operation::NO_OP);
		DESERIALIZATION_STATUS deserializeMeta(std::istream& in, const Serialization::OperationData& op, InternalDeserializationContext& dsctx);
		DESERIALIZATION_STATUS deserializeModel(std::istream& in, const Serialization::OperationData& op, InternalDeserializationContext& dsctx);
		DESERIALIZATION_STATUS deserializeMaterial(std::istream& in, const Serialization::OperationData& op, InternalDeserializationContext& dsctx);
		DESERIALIZATION_STATUS deserializeTexture(std::istream& in, const Serialization::OperationData& op, InternalDeserializationContext& dsctx);
		DESERIALIZATION_STATUS deserializeScript(std::istream& in, const Serialization::OperationData& op, InternalDeserializationContext& dsctx);
		DESERIALIZATION_STATUS deserializeCamera(std::istream& in, const Serialization::OperationData& op, InternalDeserializationContext& dsctx);
		DESERIALIZATION_STATUS deserializeGameObject(std::istream& in, const Serialization::OperationData& op, InternalDeserializationContext& dsctx);
		DESERIALIZATION_STATUS deserializeComponent(std::istream& in, const Serialization::OperationData& op, InternalDeserializationContext& dsctx);
		DESERIALIZATION_STATUS deserializeUIElement(std::istream& in, const Serialization::OperationData& op, InternalDeserializationContext& dsctx);
	};
}
