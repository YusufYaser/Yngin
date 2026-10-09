#pragma once

namespace Yngin::Services::ArchiveTools {
#pragma pack(push, 1)

	constexpr uint16_t archiveVersion = 0;

	enum class ENTRY_TYPE : uint8_t {
		NONE = 0,
		TEXTURE,
		MATERIAL,
		MODEL,
		SCRIPT,
	};

	enum class COMPRESSION_TYPE : uint8_t {
		NO_COMPRESSION = 0,
		LZ4,
	};

	struct ArchiveHeader {
		// Must always be 0x594E474E ("YNGN")
		char magic[4];
		uint16_t archiveVersion;
		uint16_t headerSize;
		uint32_t flags;

		uint16_t entrySize;
		uint64_t entriesCount;
		uint64_t totalEntriesSize;
	};

	struct ArchiveEntry {
		uint32_t id;
		char slug[33];
		ENTRY_TYPE type;
		COMPRESSION_TYPE compressionType;
		uint64_t offset;
		uint32_t compressedSize;
		uint64_t uncompressedSize;
	};

#pragma pack(pop)
}
