#pragma once
#include <Yngin/Scripting/Scripting.h>
#include <sol/sol.hpp>
#include <vector>
#include <map>

#define BIND(CLASS, NAME) #NAME, &CLASS::NAME

namespace Yngin {
	struct Script::Impl {
		Context* ctx;
		uint32_t id;
		Scene* scene;

		sol::environment env;

		void createScriptTable();
	};

	struct ScriptsManager::Impl {
		Context* ctx;
		sol::state lua;

		void bind();
		void bindGlmTypes();
		void bindYnginTypes();
		void createYnginTable();

		std::map<uint32_t, std::unique_ptr<Script>> scripts;

		void onReady();
		void onSceneActive();
		void onSceneInactive();
		void onUpdate();

		uint32_t nextId = 0;
	};
}
