#include <Yngin/Services/SerializationService.h>
#include <Yngin/Utils/Meta.h>
#include "../SerializationService_Internal.h"
#include "SerializationStructs.h"
#include <sstream>

using namespace Yngin::Services::Serialization;

namespace Yngin::Services {
	bool SerializationService::serialize(std::ostream& out, const Meta& input) {
		std::stringstream s;

		OperationData op{};
		op.schemaVersion = schemaVersion;
		op.op = Operation::META;
		op.headerSize = sizeof(SerializedMetasHeader);

		SerializedMetasHeader header{};
		header.unitInfoDataSize = sizeof(SerializedMetaInfo);

		std::map<std::string, MetaValue> metas = {};

		for (auto& [key, val] : input.getMetas()) {
			if (std::holds_alternative<void*>(val)) continue;

			bool skip = false;
			for (auto& prefix : impl->currentIgnoredMetaPrefixes) {
				if (key.starts_with(prefix)) {
					skip = true;
					break;
				}
			}
			if (skip) continue;

			header.metasCount++;
			metas[key] = val;
		}

		s.write(reinterpret_cast<const char*>(&header), op.headerSize);

		for (auto& [key, val] : metas) {
			if (std::holds_alternative<void*>(val)) continue;

			SerializedMetaInfo metaInfo{};

			metaInfo.keySize = key.length();

			void* pReal = nullptr;

			if (std::holds_alternative<std::string>(val)) {
				metaInfo.type = S_META_TYPE::STRING;
				auto pString = std::get_if<std::string>(&val);
				pReal = const_cast<char*>(pString->c_str());

				metaInfo.dataSize = pString->length();
			} else if (std::holds_alternative<int>(val)) {
				metaInfo.type = S_META_TYPE::INT32;
				auto pInt = std::get_if<int>(&val);
				pReal = pInt;

				metaInfo.dataSize = sizeof(*pInt);
			} else if (std::holds_alternative<float>(val)) {
				metaInfo.type = S_META_TYPE::FLOAT;
				auto pFloat = std::get_if<float>(&val);
				pReal = pFloat;

				metaInfo.dataSize = sizeof(*pFloat);
			}

			s.write(reinterpret_cast<const char*>(&metaInfo), header.unitInfoDataSize);
			s.write(reinterpret_cast<const char*>(key.c_str()), metaInfo.keySize);
			s.write(reinterpret_cast<const char*>(pReal), metaInfo.dataSize);
		}

		op.dataSize = s.view().size();

		out.write(reinterpret_cast<const char*>(&op), sizeof(OperationData));
		out << s.rdbuf();
		return out.good();
	}
}
