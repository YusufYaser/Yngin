#include <Yngin/Core/Context.h>
#include <string>
#include "CorePak.h"
#include <sstream>
#include "GameFiles.h"
#include "../Core/Context/Context_Internal.h"
#include <Yngin/UI/UIManager.h>
#include <Yngin/UI/Elements/UIElement.h>

#define R(name, type) s.read(reinterpret_cast<char*>(&name), sizeof(type))
#define W(name, type) s.write(reinterpret_cast<const char*>(&name), sizeof(type))

using namespace Yngin::GameFiles;
using namespace Yngin::GameFiles::CorePak;

namespace Yngin {
	bool Context::validateCorePak(const char* bytes, size_t size) {
		std::istringstream s(std::string(bytes, size), std::ios::binary);

		Header header{};
		if (!R(header, Header)) return false;
		if (std::memcmp(header.magic, "YNGINCORE", 10) != 0) return false;
		if (header.version != VERSION) return false;

		Operation op{};

		while (R(op, Operation)) {
			switch (op.op) {
			case OP::GAMEDATA:
			{
				GameData gameData;
				if (!R(gameData, GameData)) return false;

				if (!Validators::meta(s)) return false;

				break;
			}

			case OP::SCRIPT:
			{
				if (!Validators::scriptsManager(s)) return false;
				break;
			}

			case OP::UIELEMENT:
			{
				if (!Validators::uiManager(s)) return false;
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

	void Context::loadCorePak(const char* bytes, size_t size) {
		if (!validateCorePak(bytes, size)) return;

		PakLoadSettings settings = getCurrentLoadPakSettings();

		bool stop = false;

		std::istringstream s(std::string(bytes, size), std::ios::binary);

		Header header{};
		R(header, Header);
		if (std::memcmp(header.magic, "YNGINCORE", 10) != 0) return;
		if (header.version != VERSION) return;

		std::map<int, int> uiElementsParentsQueue;

		Operation op{};

		while (!stop && R(op, Operation)) {
			switch (op.op) {
			case OP::GAMEDATA:
			{
				GameData gameData;
				R(gameData, GameData);

				if (settings.applyContextSettings) {
					impl->applySettings(gameData.contextSettings);
				}

				Loaders::meta(s, meta, this);
				break;
			}

			case OP::SCRIPT:
			{
				stop = !Loaders::scriptsManager(s, impl->scriptsManager.get());
				break;
			}

			case OP::UIELEMENT:
			{
				stop = !Loaders::uiManager(s, impl->uiManager.get(), uiElementsParentsQueue);
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

		for (auto& kvp : uiElementsParentsQueue) {
			int objId = kvp.first;
			int parentId = kvp.second;
			UI::UIElement* obj = impl->uiManager->getElement(objId);
			if (obj == nullptr) continue;
			obj->setParent(parentId);
		}
	}

	std::vector<char> Context::generateCorePak() {
		ContextSettings& settings = impl->initialSettings;

		PakGenSettings genSettings = getCurrentGenPakSettings();

		if (genSettings.forceContextSettings) {
			settings = genSettings.forcedContextSettings;
		}

		std::ostringstream s(std::ios::binary);

		Header header = {};
		Operation op = {};

		std::memcpy(header.magic, "YNGINCORE", 10);
		header.version = VERSION;

		W(header, Header);

		op.op = OP::GAMEDATA;
		W(op, Operation);

		GameData gameData{};
		gameData.contextSettings = settings;

		W(gameData, GameData);

		Generators::meta(s, meta, this);

		Generators::scriptsManager(s, impl->scriptsManager.get(), nullptr);
		Generators::uiManager(s, impl->uiManager.get());

		std::string_view sv = s.view();
		return std::vector<char>(sv.begin(), sv.end());
	}
}
