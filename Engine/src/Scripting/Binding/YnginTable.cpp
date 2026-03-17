#include "../Scripting_Internal.h"
#include <Yngin/Yngin.h>

namespace Yngin {
	void ScriptsManager::Impl::createYnginTable() {
		sol::table Yngin = lua.create_table("Yngin");

		Yngin["Context"] = ctx;

		Yngin["Window"] = ctx->getWindow();

		Yngin["ScenesManager"] = ctx->getScenesManager();
		Yngin["TexturesManager"] = ctx->getTexturesManager();

		Yngin["Renderer"] = ctx->getRenderer();
		Yngin["PhysicsEngine"] = ctx->getPhysicsEngine();
		Yngin["InputSystem"] = ctx->getInputSystem();
	}
}
