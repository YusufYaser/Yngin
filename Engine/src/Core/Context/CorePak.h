#pragma once
#include <stdint.h>

namespace Yngin {
	namespace GameFiles {
		namespace CorePak {
#pragma pack(push, 1)

			struct Header {
				char magic[10];
				uint8_t version;
			};

			static_assert(sizeof(Header) == 11, "Invalid core.pak header size");

			struct GameData {
				ContextSettings contextSettings;
			};

#pragma pack(pop)
		}
	}
}