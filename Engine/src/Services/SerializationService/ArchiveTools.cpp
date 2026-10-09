#include <Yngin/Services/SerializationService.h>
#include "SerializationService_Internal.h"
#include "Serializers/SerializationStructs.h"
#include <iostream>
#include <sstream>
#include "ArchiveTools.h"
#include <lz4/lz4.h>

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
		header.headerSize = sizeof(ArchiveHeader);

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

				entry.type = ENTRY_TYPE::TEXTURE;
				entry.id = texHeader.id;

				strcpy_s(entry.slug, sizeof(entry.slug), texHeader.slug);

				in.seekg(-static_cast<std::streamoff>(op.headerSize), std::ios::cur);
			} else if (op.op == Operation::MATERIAL) {
				SerializedMaterialData matHeader{};
				in.read(reinterpret_cast<char*>(&matHeader), op.headerSize);

				entry.type = ENTRY_TYPE::MATERIAL;
				entry.id = matHeader.id;

				strcpy_s(entry.slug, sizeof(entry.slug), matHeader.slug);

				in.seekg(-static_cast<std::streamoff>(op.headerSize), std::ios::cur);
			} else if (op.op == Operation::MODEL) {
				SerializedModelData modelHeader{};
				in.read(reinterpret_cast<char*>(&modelHeader), op.headerSize);

				entry.type = ENTRY_TYPE::MODEL;
				entry.id = modelHeader.id;

				strcpy_s(entry.slug, sizeof(entry.slug), modelHeader.slug);

				in.seekg(-static_cast<std::streamoff>(op.headerSize), std::ios::cur);
			} else if (op.op == Operation::SCRIPT) {
				SerializedScriptData scriptHeader{};
				in.read(reinterpret_cast<char*>(&scriptHeader), op.headerSize);

				entry.type = ENTRY_TYPE::SCRIPT;
				entry.id = scriptHeader.id;

				strcpy_s(entry.slug, sizeof(entry.slug), scriptHeader.slug);

				in.seekg(-static_cast<std::streamoff>(op.headerSize), std::ios::cur);
			}

			in.seekg(-static_cast<std::streamoff>(sizeof(op)), std::ios::cur);

			entry.offset = data.view().size();

			entry.uncompressedSize = sizeof(op) + op.dataSize;

			if (entry.uncompressedSize > LZ4_MAX_INPUT_SIZE) throw std::runtime_error("Input too large");

			std::vector<char> inputBuffer(entry.uncompressedSize);
			in.read(inputBuffer.data(), entry.uncompressedSize);

			int maxCompressedSize = LZ4_compressBound(entry.uncompressedSize);

			std::vector<char> compressed(maxCompressedSize);

			entry.compressedSize = LZ4_compress_default(
				inputBuffer.data(),
				compressed.data(),
				entry.uncompressedSize,
				maxCompressedSize
			);

			data.write(compressed.data(), entry.compressedSize);

			entry.compressionType = COMPRESSION_TYPE::LZ4;

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

	bool SerializationService::loadArchive(std::istream& in, const DeserializationContext& deserializationContext) {
		std::streampos start = in.tellg();

		ArchiveHeader header{};

		size_t headerInfoSize = sizeof(header.magic) + sizeof(header.archiveVersion) + sizeof(header.headerSize);

		in.read(reinterpret_cast<char*>(&header), headerInfoSize);

		if (std::memcmp(header.magic, "YNGN", 4) != 0) return false;
		if (header.archiveVersion > ArchiveTools::archiveVersion) return false;

		in.seekg(start);

		in.read(reinterpret_cast<char*>(&header), header.headerSize);

		if (!impl->streamCheck(in, header.totalEntriesSize, -1)) return false;

		bool good = true;

		std::streampos nextEntryPos = in.tellg();
		for (int i = 0; i < header.entriesCount; i++) {
			in.seekg(nextEntryPos);

			ArchiveEntry entry{};
			in.read(reinterpret_cast<char*>(&entry), header.entrySize);
			nextEntryPos = in.tellg();

			in.seekg(start + static_cast<std::streampos>(entry.offset));

			if (entry.compressedSize > INT_MAX || entry.uncompressedSize > LZ4_MAX_INPUT_SIZE)
				continue;

			std::vector<char> compressed(entry.compressedSize);
			in.read(compressed.data(), entry.compressedSize);

			std::vector<char> uncompressed(entry.uncompressedSize);

			int decompressedSize = LZ4_decompress_safe(
				compressed.data(),
				uncompressed.data(),
				entry.compressedSize,
				entry.uncompressedSize
			);

			if (entry.uncompressedSize != static_cast<uint64_t>(decompressedSize)) {
				good = false;
				continue;
			}

			std::stringstream dataStream(std::string(uncompressed.data(), uncompressed.size()));

			auto deserializationStatus = load(dataStream, deserializationContext);

			good = good && (deserializationStatus == DESERIALIZATION_STATUS::OK);
		}

		return good;
	}
}