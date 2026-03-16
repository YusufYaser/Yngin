#pragma once
#include <Yngin/Scripting/Scripting.h>
#include <sol/sol.hpp>
#include <vector>
#include <map>

namespace Yngin {
	struct Script::Impl {
		Context* ctx;
		uint32_t id;

		sol::environment env;
	};

	struct ScriptsManager::Impl {
		Context* ctx;

		sol::state lua;
		std::map<uint32_t, std::unique_ptr<Script>> scripts;

		void onReady();
		void onUpdate();

		uint32_t nextId = 0;
	};
}
