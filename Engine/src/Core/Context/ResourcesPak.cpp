#include <Yngin/Core/Context.h>
#include "ResourcesPak.h"
#include "Context_Internal.h"
#include "../Scripting/Scripting_Internal.h"
#include <Yngin/Core/Models.h>
#include <Yngin/Rendering/Textures.h>
#include <Yngin/Core/Scripting.h>
#include <Yngin/Core/Scenes.h>
#include <sstream>
#include <glm/gtc/type_ptr.hpp>
#include <glad/glad.h>
#include <stb/stb_image.h>
#include <stb/stb_image_write.h>
#include "../../GameFiles/GameFiles.h"

#define R(name, type) s.read(reinterpret_cast<char*>(&name), sizeof(type))
#define W(name, type) s.write(reinterpret_cast<const char*>(&name), sizeof(type))

using namespace Yngin::GameFiles;
using namespace Yngin::GameFiles::ResourcesPak;

namespace Yngin {
	void Context::loadResourcesPak(const char* bytes, size_t size) {
		bool stop = false;

		std::istringstream s(std::string(bytes, size), std::ios::binary);

		Header header{};
		R(header, Header);
		if (std::memcmp(header.magic, "YNGINRESOURCES", 14) != 0) return;
		if (header.version != VERSION) return;

		Operation op{};

		while (!stop && R(op, Operation)) {
			switch (op.op) {
			case OP::MODEL:
			{
				stop = !Loaders::modelsManager(s, impl->modelsManager.get());
				break;
			}

			case OP::TEXTURE:
			{
				stop = !Loaders::texturesManager(s, impl->texturesManager.get());
				break;
			}

			case OP::SCRIPT:
			{
				stop = !Loaders::scriptsManager(s, impl->scriptsManager.get());
				break;
			}

			default:
			{
				stop = true;
				break;
			}
			}
		}

		if (stop) return;
	}

	std::vector<char> Context::generateResourcesPak() {
		std::ostringstream s(std::ios::binary);

		Header header = {};
		Operation op = {};

		std::memcpy(header.magic, "YNGINRESOURCES", 14);
		header.version = VERSION;

		W(header, Header);

		Generators::modelsManager(s, impl->modelsManager.get());
		Generators::texturesManager(s, impl->texturesManager.get());
		Generators::scriptsManager(s, impl->scriptsManager.get(), nullptr);

		std::string_view sv = s.view();
		return std::vector<char>(sv.begin(), sv.end());
	}
}
