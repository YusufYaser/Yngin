#include <Yngin/Services/SerializationService.h>
#include "SerializationService_Internal.h"
#include "Serializers/SerializationStructs.h"
#include <iostream>
#include <sstream>
#include "ArchiveTools.h"
#include <lz4/lz4.h>
#include <Yngin/Rendering/Textures.h>
#include "../../Rendering/Textures/Textures_Internal.h"
#include <Yngin/Core/Materials.h>
#include "../../Core/Materials/Materials_Internal.h"
#include <Yngin/Core/Models.h>
#include "../../Core/Models/Models_Internal.h"

#define LOGGER_NAME SerializationService
#include "../../Internal/Logger.h"

using namespace Yngin::Services::ArchiveTools;
using namespace Yngin::Services::Serialization;

namespace Yngin::Services {
	std::vector<char> SerializationService::createArchive(std::istream& in, const CreateArchiveSettings& settings) {
		std::streampos originalPos = in.tellg();
		if (validate(in) != DESERIALIZATION_STATUS::OK) {
			DEBUG("Cannot create archive from invalid serialized data");
			return {};
		}
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

		DEBUG("Creating archive from serialized data");
		while (in.good() && !in.eof()) {
			OperationData op{};

			in.read(reinterpret_cast<char*>(&op.schemaVersion), sizeof(uint16_t));

			if (op.schemaVersion > Serialization::schemaVersion) return {};

			in.seekg(-2, std::ios::cur);

			in.read(reinterpret_cast<char*>(&op), sizeof(OperationData));

			if (op.op != Operation::TEXTURE && op.op != Operation::MATERIAL && op.op != Operation::MODEL && op.op != Operation::SCRIPT) {
				TRACE("Skipping unsupported operation: %i", (int)op.op);
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

			TRACE("Added entry to archive: type=%i, id=%i, slug=%s, offset=%llu, compressedSize=%u, uncompressedSize=%llu",
				(int)entry.type, entry.id, entry.slug, entry.offset, entry.compressedSize, entry.uncompressedSize);
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

		DEBUG("Created a new archive with %lli entries and total size %zu", header.entriesCount, archive.size());

		return archive;
	}

	bool SerializationService::loadArchive(std::istream& in, const DeserializationContext& deserializationContext) {
		std::streampos start = in.tellg();

		ArchiveHeader header{};

		size_t headerInfoSize = sizeof(header.magic) + sizeof(header.archiveVersion) + sizeof(header.headerSize);

		in.read(reinterpret_cast<char*>(&header), headerInfoSize);

		if (std::memcmp(header.magic, "YNGN", 4) != 0) {
			DEBUG("Invalid archive magic number");
			return false;
		}
		if (header.archiveVersion > ArchiveTools::archiveVersion) {
			DEBUG("Unsupported archive version: %u", header.archiveVersion);
			return false;
		}

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

			good = good && impl->loadArchiveEntry(in, start, entry, deserializationContext);
		}

		DEBUG("Loaded %lli entries from archive with status: %s", header.entriesCount, good ? "OK" : "ERROR");

		return good;
	}

	bool SerializationService::Impl::loadArchiveEntry(std::istream& in, const std::streampos& start, const ArchiveEntry& entry, const DeserializationContext& deserializationContext) {
		in.seekg(start + static_cast<std::streampos>(entry.offset));

		if (!streamCheck(in, entry.compressedSize, -1)) {
			TRACE("Skipping invalid entry data: entryType=%u, entryId=%u, compressedSize=%u", (int)entry.type, entry.id, entry.compressedSize);
			return false;
		}

		if (entry.compressedSize > INT_MAX || entry.uncompressedSize > LZ4_MAX_INPUT_SIZE) {
			TRACE("Skipping invalid size data: entryType=%u, entryId=%u, compressedSize=%u, uncompressedSize=%llu", (int)entry.type, entry.id, entry.compressedSize, entry.uncompressedSize);
			return false;
		}

		std::vector<char> compressed(entry.compressedSize);
		in.read(compressed.data(), entry.compressedSize);

		std::vector<char> uncompressed(entry.uncompressedSize);

		if (entry.compressionType == COMPRESSION_TYPE::NO_COMPRESSION) {
			if (entry.compressedSize != entry.uncompressedSize) {
				TRACE("Skipping invalid sizes data for uncompressed entry: entryType=%u, entryId=%u, compressedSize=%u, uncompressedSize=%llu", (int)entry.type, entry.id, entry.compressedSize, entry.uncompressedSize);
				return false;
			}
			// copy compressed to uncompressed
			std::memcpy(uncompressed.data(), compressed.data(), entry.compressedSize);
		} else if (entry.compressionType == COMPRESSION_TYPE::LZ4) {
			int decompressedSize = LZ4_decompress_safe(
				compressed.data(),
				uncompressed.data(),
				entry.compressedSize,
				entry.uncompressedSize
			);

			if (entry.uncompressedSize != static_cast<uint64_t>(decompressedSize)) {
				return false;
			}
		} else {
			TRACE("Skipping unsupported compression type: %i", (int)entry.compressionType);
			return false;
		}

		std::stringstream dataStream(std::string(uncompressed.data(), uncompressed.size()));

		auto deserializationStatus = owner->load(dataStream, deserializationContext);

		return deserializationStatus == DESERIALIZATION_STATUS::OK;
	}

	uint32_t SerializationService::streamArchive(std::unique_ptr<std::istream>& archive, const DeserializationContext& userdsctx) {
		std::istream& in = *archive;
		std::streampos start = in.tellg();

		ArchiveHeader header{};

		size_t headerInfoSize = sizeof(header.magic) + sizeof(header.archiveVersion) + sizeof(header.headerSize);

		in.read(reinterpret_cast<char*>(&header), headerInfoSize);

		if (std::memcmp(header.magic, "YNGN", 4) != 0) {
			DEBUG("Invalid archive magic number");
			return 0;
		}
		if (header.archiveVersion > ArchiveTools::archiveVersion) {
			DEBUG("Unsupported archive version: %u", header.archiveVersion);
			return 0;
		}

		in.seekg(start);

		in.read(reinterpret_cast<char*>(&header), header.headerSize);

		if (!impl->streamCheck(in, header.totalEntriesSize, -1)) return 0;

		StreamContext* streamContext = new StreamContext();
		streamContext->archive = std::move(archive);
		streamContext->archiveStart = start;
		uint32_t id = impl->nextStreamId++;
		impl->streams[id] = std::unique_ptr<StreamContext>(streamContext);

		DEBUG("Creating streamable objects");

		std::streampos nextEntryPos = in.tellg();
		for (int i = 0; i < header.entriesCount; i++) {
			in.seekg(nextEntryPos);

			ArchiveEntry entry{};
			in.read(reinterpret_cast<char*>(&entry), header.entrySize);

			nextEntryPos = in.tellg();

			in.seekg(start + static_cast<std::streampos>(entry.offset));

			if (!impl->streamCheck(in, entry.compressedSize, -1)) {
				TRACE("Skipping invalid entry data: entryType=%u, entryId=%u, compressedSize=%u", (int)entry.type, entry.id, entry.compressedSize);
				continue;
			}

			if (entry.compressedSize > INT_MAX || entry.uncompressedSize > LZ4_MAX_INPUT_SIZE) {
				TRACE("Skipping invalid size data: entryType=%u, entryId=%u, compressedSize=%u, uncompressedSize=%llu", (int)entry.type, entry.id, entry.compressedSize, entry.uncompressedSize);
				continue;
			}

			if (entry.type != ENTRY_TYPE::TEXTURE && entry.type != ENTRY_TYPE::MATERIAL && entry.type != ENTRY_TYPE::MODEL && entry.type != ENTRY_TYPE::SCRIPT) {
				TRACE("Skipping unsupported entry type: %i", (int)entry.type);
				continue;
			}

			TRACE("Creating object type=%i with id=%u to be loaded on demand", (int)entry.type, entry.id);

			if (entry.type == ENTRY_TYPE::TEXTURE) {
				if (!userdsctx.overrideConflictingId && impl->ctx->getTexturesManager()->getTexture(entry.id)) {
					TRACE("Skipping texture with conflicting id=%u", entry.id);
					continue;
				}

				Texture* texture = impl->ctx->getTexturesManager()->createTexture(entry.id, true);

				texture->impl->loadOnDemand = true;
				texture->impl->streamId = id;

				StreamableObjectData streamable{};
				streamable.type = StreamableObjectType::TEXTURE;
				streamable.objectId = entry.id;
				streamable.archiveEntry = entry;

				streamContext->objects[(size_t)streamable.type][streamable.objectId] = streamable;
			} else if (entry.type == ENTRY_TYPE::MATERIAL) {
				if (!userdsctx.overrideConflictingId && impl->ctx->getMaterialsManager()->getMaterial(entry.id)) {
					TRACE("Skipping material with conflicting id=%u", entry.id);
					continue;
				}

				Material* material = impl->ctx->getMaterialsManager()->createMaterial(entry.id, true);

				material->impl->loadOnDemand = true;
				material->impl->streamId = id;

				StreamableObjectData streamable{};
				streamable.type = StreamableObjectType::MATERIAL;
				streamable.objectId = entry.id;
				streamable.archiveEntry = entry;

				streamContext->objects[(size_t)streamable.type][streamable.objectId] = streamable;
			} else if (entry.type == ENTRY_TYPE::MODEL) {
				if (!userdsctx.overrideConflictingId && impl->ctx->getModelsManager()->getModel(entry.id)) {
					TRACE("Skipping model with conflicting id=%u", entry.id);
					continue;
				}

				Model* model = impl->ctx->getModelsManager()->createModel({}, entry.id, true);

				model->impl->loadOnDemand = true;
				model->impl->streamId = id;

				StreamableObjectData streamable{};
				streamable.type = StreamableObjectType::MODEL;
				streamable.objectId = entry.id;
				streamable.archiveEntry = entry;

				streamContext->objects[(size_t)streamable.type][streamable.objectId] = streamable;
			} else if (entry.type == ENTRY_TYPE::SCRIPT) {
				TRACE("Loading script with id=%u now", entry.id);
				bool status = impl->loadArchiveEntry(*streamContext->archive, streamContext->archiveStart, entry, userdsctx);
				if (!status) {
					TRACE("Failed to load script with id=%u", entry.id);
				}
			}

			// TODO: load meta data
		}


		return id;
	}

	bool SerializationService::Impl::notifyObjectRemoved(uint32_t streamId, const StreamableObjectType& type, uint32_t objectId) {
		TRACE("Notifying object removed from stream: streamId=%u, type=%u, objectId=%u", streamId, (uint8_t)type, objectId);

		if (type == StreamableObjectType::UNKNOWN || type >= StreamableObjectType::COUNT) {
			return false;
		}

		auto it = streams.find(streamId);
		if (it == streams.end()) {
			TRACE("Stream id %u not found", streamId);
			return false;
		}

		StreamContext* streamContext = it->second.get();
		auto& objects = streamContext->objects[(size_t)type];
		auto objIt = objects.find(objectId);
		if (objIt == objects.end()) {
			TRACE("Object id %u not found in stream %u", objectId, streamId);
			return false;
		}

		DEBUG("Removing object from stream: streamId=%u, type=%u, objectId=%u", streamId, (uint8_t)type, objectId);
		objects.erase(objectId);

		return true;
	}

	bool SerializationService::Impl::notifyDataAccessed(uint32_t streamId, const StreamableObjectType& type, uint32_t objectId) {
		switch (type) {
		case StreamableObjectType::TEXTURE: {
			Texture* obj = ctx->getTexturesManager()->getTexture(objectId);
			if (obj && obj->impl->loadOnDemand) {
				if (!obj->impl->dataLoadedOnDemand) {
					obj->impl->loadOnDemand = false;
					obj->impl->dataLoadedOnDemand = loadData(streamId, type, objectId);
					obj->impl->loadOnDemand = true;
				}
				return obj->impl->dataLoadedOnDemand;
			}
			break;
		}

		case StreamableObjectType::MATERIAL: {
			Material* obj = ctx->getMaterialsManager()->getMaterial(objectId);
			if (obj && obj->impl->loadOnDemand) {
				if (!obj->impl->dataLoadedOnDemand) {
					obj->impl->loadOnDemand = false;
					obj->impl->dataLoadedOnDemand = loadData(streamId, type, objectId);
					obj->impl->loadOnDemand = true;
				}
				return obj->impl->dataLoadedOnDemand;
			}
			break;
		}

		case StreamableObjectType::MODEL: {
			Model* obj = ctx->getModelsManager()->getModel(objectId);
			if (obj && obj->impl->loadOnDemand) {
				if (!obj->impl->dataLoadedOnDemand) {
					obj->impl->loadOnDemand = false;
					obj->impl->dataLoadedOnDemand = loadData(streamId, type, objectId);
					obj->impl->loadOnDemand = true;
				}
				return obj->impl->dataLoadedOnDemand;
			}
			break;
		}

		default:
			return false;
		}
		return true;
	}

	bool SerializationService::Impl::notifyDataModified(uint32_t streamId, const StreamableObjectType& type, uint32_t objectId) {
		return notifyDataAccessed(streamId, type, objectId);
	}

	bool SerializationService::Impl::loadData(uint32_t streamId, const StreamableObjectType& type, uint32_t objectId) {
		DEBUG("Requesting stream data: streamId=%u, type=%u, objectId=%u", streamId, (uint8_t)type, objectId);

		if (type == StreamableObjectType::UNKNOWN || type >= StreamableObjectType::COUNT) {
			DEBUG("Invalid streamable object type: %u", (uint8_t)type);
			return false;
		}

		auto it = streams.find(streamId);
		if (it == streams.end()) {
			DEBUG("Stream id %u not found", streamId);
			return false;
		}

		StreamContext* streamContext = it->second.get();
		auto& objects = streamContext->objects[(size_t)type];
		auto objIt = objects.find(objectId);
		if (objIt == objects.end()) {
			DEBUG("Object id %u not found in stream %u", objectId, streamId);
			return false;
		}
		StreamableObjectData& streamable = objIt->second;

		const ArchiveEntry& entry = streamable.archiveEntry;

		DeserializationContext deserializationContext{};
		deserializationContext.useOriginalConflictingObject = true;

		bool status = loadArchiveEntry(*streamContext->archive, streamContext->archiveStart, entry, deserializationContext);

		if (status) {
			DEBUG("Successfully loaded stream data: streamId=%u, type=%u, objectId=%u", streamId, (uint8_t)type, objectId);
		} else {
			DEBUG("Failed to load stream data: streamId=%u, type=%u, objectId=%u", streamId, (uint8_t)type, objectId);
		}

		return status;
	}
}