#include <Yngin/Services/SerializationService.h>
#include "SerializationService_Internal.h"
#include "Serializers/SerializationStructs.h"
#include <iostream>
#include <sstream>
#include "ArchiveTools.h"

using namespace Yngin::Services::ArchiveTools;
using namespace Yngin::Services::Serialization;

namespace Yngin::Services {
	std::vector<char> SerializationService::createArchive(std::istream& in, const CreateArchiveSettings& settings) {
		std::streampos originalPos = in.tellg();
		if (validate(in) != DESERIALIZATION_STATUS::OK) return {};
		in.seekg(originalPos);

		std::stringstream s;

		ArchiveHeader header{};
		header.magic[0] = 'Y';
		header.magic[1] = 'N';
		header.magic[2] = 'G';
		header.magic[3] = 'N';
		header.archiveVersion = ArchiveTools::archiveVersion;

		header.entrySize = sizeof(ArchiveEntry);

		std::vector<ArchiveEntry> entries;

		std::stringstream data;

		while (in.good() && !in.eof()) {
			OperationData op{};

			in.read(reinterpret_cast<char*>(&op.schemaVersion), sizeof(uint16_t));

			if (op.schemaVersion > Serialization::schemaVersion) return {};

			in.seekg(-2, std::ios::cur);

			in.read(reinterpret_cast<char*>(&op), sizeof(OperationData));

			if (op.op != Operation::TEXTURE && op.op != Operation::MATERIAL && op.op != Operation::MODEL && op.op != Operation::SCRIPT) {
				in.seekg(op.dataSize, std::ios::cur);
				continue;
			}

			ArchiveEntry entry{};

			if (op.op == Operation::TEXTURE) {
				SerializedTextureData texHeader{};
				in.read(reinterpret_cast<char*>(&texHeader), op.headerSize);

				entry.id = texHeader.id;

				strcpy_s(entry.slug, sizeof(entry.slug), texHeader.slug);

				in.seekg(-static_cast<std::streamoff>(op.headerSize), std::ios::cur);
			} else if (op.op == Operation::MATERIAL) {
				SerializedMaterialData matHeader{};
				in.read(reinterpret_cast<char*>(&matHeader), op.headerSize);

				entry.id = matHeader.id;

				strcpy_s(entry.slug, sizeof(entry.slug), matHeader.slug);

				in.seekg(-static_cast<std::streamoff>(op.headerSize), std::ios::cur);
			} else if (op.op == Operation::MODEL) {
				SerializedModelData modelHeader{};
				in.read(reinterpret_cast<char*>(&modelHeader), op.headerSize);

				entry.id = modelHeader.id;

				strcpy_s(entry.slug, sizeof(entry.slug), modelHeader.slug);

				in.seekg(-static_cast<std::streamoff>(op.headerSize), std::ios::cur);
			} else if (op.op == Operation::SCRIPT) {
				SerializedScriptData scriptHeader{};
				in.read(reinterpret_cast<char*>(&scriptHeader), op.headerSize);

				entry.id = scriptHeader.id;

				strcpy_s(entry.slug, sizeof(entry.slug), scriptHeader.slug);

				in.seekg(-static_cast<std::streamoff>(op.headerSize), std::ios::cur);
			}

			entry.offset = data.view().size();

			std::vector<char> entryData(op.dataSize);
			in.read(entryData.data(), op.dataSize);
			data.write(reinterpret_cast<const char*>(&op), sizeof(op));
			data.write(entryData.data(), op.dataSize);

			entry.uncompressedSize = data.view().size() - entry.offset;
			entry.compressedSize = entry.uncompressedSize;

			entries.push_back(entry);
		}

		header.entriesCount = entries.size();
		header.totalEntriesSize = entries.size() * sizeof(ArchiveEntry);

		for (size_t i = 0; i < entries.size(); i++) {
			entries[i].offset += sizeof(ArchiveHeader) + header.totalEntriesSize;
		}

		s.write(reinterpret_cast<const char*>(&header), sizeof(ArchiveHeader));
		s.write(reinterpret_cast<const char*>(entries.data()), header.totalEntriesSize);
		s << data.rdbuf();

		auto archive = std::vector<char>(s.view().begin(), s.view().end());

		return archive;
	}
}