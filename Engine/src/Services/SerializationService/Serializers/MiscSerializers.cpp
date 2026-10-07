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

		s.write(reinterpret_cast<const char*>(&header), sizeof(SerializedMetasHeader));

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
		if (!s.view().empty()) out << s.rdbuf();
		return out.good();
	}

	DESERIALIZATION_STATUS SerializationService::Impl::validateMeta(std::istream& in, const Serialization::OperationData& op) {
		SerializedMetasHeader header{};

		if (!streamCheck(in, op.headerSize, sizeof(SerializedMetasHeader))) return DESERIALIZATION_STATUS::INVALID_DATA;
		in.read(reinterpret_cast<char*>(&header), op.headerSize);

		for (int i = 0; i < header.metasCount; i++) {
			SerializedMetaInfo info{};

			if (!streamCheck(in, header.unitInfoDataSize, sizeof(SerializedMetaInfo))) return DESERIALIZATION_STATUS::INVALID_DATA;
			in.read(reinterpret_cast<char*>(&info), header.unitInfoDataSize);

			switch (info.type) {
			case S_META_TYPE::INT32:
			case S_META_TYPE::FLOAT:
			case S_META_TYPE::STRING:
			{
				if (!streamCheck(in, info.keySize, -1)) return DESERIALIZATION_STATUS::INVALID_DATA;
				in.seekg(info.keySize, std::ios::cur);
				if (!streamCheck(in, info.dataSize, -1)) return DESERIALIZATION_STATUS::INVALID_DATA;
				in.seekg(info.dataSize, std::ios::cur);

				break;
			}

			default: // including a pointer meta
				return DESERIALIZATION_STATUS::INVALID_DATA;
			}
		}

		return DESERIALIZATION_STATUS::OK;
	}

	DESERIALIZATION_STATUS SerializationService::Impl::deserializeMeta(std::istream& in, const Serialization::OperationData& op, InternalDeserializationContext& dsctx) {
		if (dsctx.meta == nullptr) {
			// Skip the operation data in case we ignore missing context errors
			if (!streamCheck(in, op.dataSize, -1)) return DESERIALIZATION_STATUS::GENERIC_ERROR;
			in.seekg(op.dataSize, std::ios::cur);
			return DESERIALIZATION_STATUS::MISSING_CONTEXT;
		}

		SerializedMetasHeader header{};

		if (!streamCheck(in, op.headerSize, sizeof(SerializedMetasHeader))) return DESERIALIZATION_STATUS::INVALID_DATA;
		in.read(reinterpret_cast<char*>(&header), op.headerSize);

		for (int i = 0; i < header.metasCount; i++) {
			SerializedMetaInfo info{};

			if (!streamCheck(in, header.unitInfoDataSize, sizeof(SerializedMetaInfo))) return DESERIALIZATION_STATUS::INVALID_DATA;
			in.read(reinterpret_cast<char*>(&info), header.unitInfoDataSize);

			if (!streamCheck(in, info.keySize, -1)) return DESERIALIZATION_STATUS::INVALID_DATA;
			std::vector<unsigned char> keyBuffer(info.keySize);
			in.read(reinterpret_cast<char*>(keyBuffer.data()), info.keySize);

			switch (info.type) {
			case S_META_TYPE::INT32:
			{
				uint32_t u32 = 0;

				if (!streamCheck(in, info.dataSize, sizeof(u32))) return DESERIALIZATION_STATUS::INVALID_DATA;
				in.read(reinterpret_cast<char*>(&u32), info.dataSize);

				dsctx.meta->setMeta(std::string(keyBuffer.begin(), keyBuffer.end()), int(u32));
				break;
			}
			case S_META_TYPE::FLOAT:
			{
				float f = 0.0f;
				if (!streamCheck(in, info.dataSize, sizeof(f))) return DESERIALIZATION_STATUS::INVALID_DATA;
				in.read(reinterpret_cast<char*>(&f), info.dataSize);
				dsctx.meta->setMeta(std::string(keyBuffer.begin(), keyBuffer.end()), f);
				break;
			}

			case S_META_TYPE::STRING:
			{
				if (!streamCheck(in, info.dataSize, -1)) return DESERIALIZATION_STATUS::INVALID_DATA;
				std::vector<unsigned char> dataBuffer(info.dataSize);
				in.read(reinterpret_cast<char*>(dataBuffer.data()), info.dataSize);
				dsctx.meta->setMeta(std::string(keyBuffer.begin(), keyBuffer.end()), std::string(dataBuffer.begin(), dataBuffer.end()));
				break;
			}

			default: // including a pointer meta
				return DESERIALIZATION_STATUS::INVALID_DATA;
			}
		}

		return DESERIALIZATION_STATUS::OK;
	}
}
