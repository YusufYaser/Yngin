#pragma once
#include <stdint.h>

namespace Yngin {
	namespace GameFiles {
		namespace GamePak {
#pragma pack(push, 1)

			struct Header {
				char magic[10];
				uint8_t version;
			};

			static_assert(sizeof(Header) == 11, "Invalid game.pak header size");

			enum class PAK_TYPE : uint8_t {
				CORE,
				RESOURCES,
				SCENE
			};

			struct PakInfo {
				PAK_TYPE pakType;
				int linkedId = -1;
				size_t pakSize;
			};

#pragma pack(pop)
		}
	}
}