#pragma once
#include <Yngin/Services/SerializationService.h>
#include <Yngin/Utils/Meta.h>
#include "Serializers/SerializationStructs.h"

namespace Yngin::Services {
	struct SerializationService::Impl {
		std::vector<std::vector<std::string>> skipMetaKeys;
		std::vector<std::string> currentSkipMetaKeys;
		std::vector<std::vector<std::string>> ignoredMetaPrefixes;
		std::vector<std::string> currentIgnoredMetaPrefixes;

		bool shouldSkip(const Meta& meta);


		// Validators
		bool streamCheck(std::istream& in, size_t size);
		bool validateOperation(std::istream& in, const Serialization::Operation& checkOp = Serialization::Operation::NO_OP);
		bool validateMeta(std::istream& in, const Serialization::OperationData& op);
		bool validateModel(std::istream& in, const Serialization::OperationData& op);
		bool validateMaterial(std::istream& in, const Serialization::OperationData& op);
		bool validateTexture(std::istream& in, const Serialization::OperationData& op);
		bool validateScript(std::istream& in, const Serialization::OperationData& op);
		bool validateCamera(std::istream& in, const Serialization::OperationData& op);
		bool validateGameObject(std::istream& in, const Serialization::OperationData& op);
		bool validateComponent(std::istream& in, const Serialization::OperationData& op);
		bool validateUIElement(std::istream& in, const Serialization::OperationData& op);
	};
}
