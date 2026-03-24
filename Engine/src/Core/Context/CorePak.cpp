#include <Yngin/Core/Context.h>
#include <string>
#include "CorePak.h"
#include <sstream>
#include "../../GameFiles/GameFiles.h"
#include "Context_Internal.h"
#include <Yngin/UI/UIManager.h>
#include <Yngin/UI/Elements/UIElement.h>

#define R(name, type) s.read(reinterpret_cast<char*>(&name), sizeof(type))
#define W(name, type) s.write(reinterpret_cast<const char*>(&name), sizeof(type))

using namespace Yngin::GameFiles;
using namespace Yngin::GameFiles::CorePak;

namespace Yngin {
	void Context::loadCorePak(const char* bytes, size_t size) {
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
				impl->applySettings(gameData.contextSettings);
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
		return generateCorePak(impl->initialSettings);
	}

	std::vector<char> Context::generateCorePak(const ContextSettings& settings) {
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

		Generators::scriptsManager(s, impl->scriptsManager.get(), nullptr);
		Generators::uiManager(s, impl->uiManager.get());

		std::string_view sv = s.view();
		return std::vector<char>(sv.begin(), sv.end());
	}
}
