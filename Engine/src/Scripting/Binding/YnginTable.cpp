#include "../Scripting_Internal.h"
#include <Yngin/Yngin.h>

namespace Yngin {
	void ScriptsManager::Impl::createYnginTable() {
		sol::table Yngin = lua.create_table("Yngin");

		Yngin["Context"] = ctx;
		Yngin["InputSystem"] = ctx->getInputSystem();
		Yngin["Window"] = ctx->getWindow();
	}
}
