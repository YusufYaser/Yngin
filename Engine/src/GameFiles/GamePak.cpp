#include <Yngin/Core/Context.h>
#include <string>
#include "GamePak.h"
#include <sstream>
#include "GameFiles.h"
#include "../Core/Context/Context_Internal.h"
#include <Yngin/Core/Scenes.h>

#define R(name, type) s.read(reinterpret_cast<char*>(&name), sizeof(type))
#define W(name, type) s.write(reinterpret_cast<const char*>(&name), sizeof(type))

using namespace Yngin::GameFiles;
using namespace Yngin::GameFiles::GamePak;

namespace Yngin {
	namespace GameFiles {
		const int VERSION = 4;
	}

	void Context::pushLoadPakSettings(const PakLoadSettings& settings) {
		impl->pakLoadSettings.push_back(settings);
	}

	void Context::popLoadPakSettings() {
		impl->pakLoadSettings.pop_back();
	}

	PakLoadSettings Context::getCurrentLoadPakSettings() const {
		PakLoadSettings settings{};

		if (!impl->pakLoadSettings.empty()) {
			settings = impl->pakLoadSettings.back();
		}

		return settings;
	}

	void Context::pushGenPakSettings(const PakGenSettings& settings) {
		impl->pakGenSettings.push_back(settings);
	}

	void Context::popGenPakSettings() {
		impl->pakGenSettings.pop_back();
	}

	PakGenSettings Context::getCurrentGenPakSettings() const {
		PakGenSettings settings{};

		if (!impl->pakGenSettings.empty()) {
			settings = impl->pakGenSettings.back();
		}

		return settings;
	}

	void Context::loadGamePak(const char* bytes, size_t size) {
		std::istringstream s(std::string(bytes, size), std::ios::binary);

		Header header{};
		R(header, Header);
		if (std::memcmp(header.magic, "YNGINGAME", 10) != 0) return;
		if (header.version != VERSION) return;

		std::map<int, int> uiElementsParentsQueue;

		PakInfo pakInfo{};

		while (R(pakInfo, PakInfo)) {
			char* bytes = new char[pakInfo.pakSize];
			s.read(bytes, pakInfo.pakSize);

			switch (pakInfo.pakType) {
			case PAK_TYPE::CORE:
			{
				loadCorePak(bytes, pakInfo.pakSize);
				break;
			}

			case PAK_TYPE::RESOURCES:
			{
				loadResourcesPak(bytes, pakInfo.pakSize);
				break;
			}

			case PAK_TYPE::SCENE:
			{
				// TODO: don't load all scenes at once
				impl->scenesManager->createScene(bytes, pakInfo.pakSize, pakInfo.linkedId, true);
				break;
			}

			default:
			{
				break;
			}
			}

			delete[] bytes;
		}
	}

	std::vector<char> Context::generateGamePak() {
		std::ostringstream s(std::ios::binary);

		Header header = {};
		Operation op = {};

		std::memcpy(header.magic, "YNGINGAME", 10);
		header.version = VERSION;

		W(header, Header);

		PakInfo pakInfo{};

		pakInfo.pakType = PAK_TYPE::CORE;
		auto corePakBytes = generateCorePak();
		pakInfo.pakSize = corePakBytes.size();
		W(pakInfo, PakInfo);
		s.write(corePakBytes.data(), pakInfo.pakSize);


		pakInfo.pakType = PAK_TYPE::RESOURCES;
		auto resoucresPakBytes = generateResourcesPak();
		pakInfo.pakSize = resoucresPakBytes.size();
		W(pakInfo, PakInfo);
		s.write(resoucresPakBytes.data(), pakInfo.pakSize);


		for (Scene* scene : impl->scenesManager->getScenes()) {
			if (scene->meta.getMetaInt("#NoExport", 0)) break;

			pakInfo.pakType = PAK_TYPE::SCENE;
			pakInfo.linkedId = scene->getId();
			auto scenePakBytes = scene->generatePak();
			pakInfo.pakSize = scenePakBytes.size();
			W(pakInfo, PakInfo);
			s.write(scenePakBytes.data(), pakInfo.pakSize);
		}
		pakInfo.linkedId = -1;

		std::string_view sv = s.view();
		return std::vector<char>(sv.begin(), sv.end());
	}
}