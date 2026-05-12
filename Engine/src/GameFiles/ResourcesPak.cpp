#include <Yngin/Core/Context.h>
#include "ResourcesPak.h"
#include "../Core/Context/Context_Internal.h"
#include "../Core/Scripting/Scripting_Internal.h"
#include <Yngin/Core/Models.h>
#include <Yngin/Rendering/Textures.h>
#include <Yngin/Core/Scripting.h>
#include <Yngin/Core/Scenes.h>
#include <sstream>
#include <glm/gtc/type_ptr.hpp>
#include <glad/glad.h>
#include <stb/stb_image.h>
#include <stb/stb_image_write.h>
#include "GameFiles.h"

#define LOGGER_NAME PakSerializer
#include "../Internal/Logger.h"

#define R(name, type) s.read(reinterpret_cast<char*>(&name), sizeof(type))
#define W(name, type) s.write(reinterpret_cast<const char*>(&name), sizeof(type))

using namespace Yngin::GameFiles;
using namespace Yngin::GameFiles::ResourcesPak;

namespace Yngin {
	bool Context::validateResourcesPak(const char* bytes, size_t size) {
		std::istringstream s(std::string(bytes, size), std::ios::binary);

		Header header{};
		if (!R(header, Header)) return false;
		if (std::memcmp(header.magic, "YNGINRESOURCES", 14) != 0) return false;
		if (header.version != VERSION) return false;

		Operation op{};

		while (R(op, Operation)) {
			switch (op.op) {
			case OP::MODEL:
			{
				if (!Validators::modelsManager(s)) return false;
				break;
			}

			case OP::TEXTURE:
			{
				if (!Validators::texturesManager(s)) return false;
				break;
			}

			case OP::MATERIAL:
			{
				if (!Validators::materialsManager(s)) return false;
				break;
			}

			default:
			{
				return false;
			}
			}
		}

		return true;
	}

	void Context::loadResourcesPak(const char* bytes, size_t size) {
		if (!validateResourcesPak(bytes, size)) return;

		bool stop = false;

		std::istringstream s(std::string(bytes, size), std::ios::binary);

		Header header{};
		R(header, Header);
		if (std::memcmp(header.magic, "YNGINRESOURCES", 14) != 0) return;
		if (header.version != VERSION) return;

		DEBUG("Deserializing resources pak to context");

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

			case OP::MATERIAL:
			{
				stop = !Loaders::materialsManager(s, impl->materialsManager.get());
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

		DEBUG("Deserialized resources pak to context successfully");
	}

	std::vector<char> Context::generateResourcesPak() {
		DEBUG("Serializing context resources into a resources pak");

		std::ostringstream s(std::ios::binary);

		Header header = {};
		Operation op = {};

		std::memcpy(header.magic, "YNGINRESOURCES", 14);
		header.version = VERSION;

		W(header, Header);

		Generators::modelsManager(s, impl->modelsManager.get());
		Generators::texturesManager(s, impl->texturesManager.get());
		Generators::materialsManager(s, impl->materialsManager.get());

		DEBUG("Serialized context resources into a resources pak successfully");

		std::string_view sv = s.view();
		return std::vector<char>(sv.begin(), sv.end());
	}
}
