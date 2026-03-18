#include <Yngin/Scripting/Scripting.h>
#include <sol/sol.hpp>
#include "Scripting_Internal.h"
#include <Yngin/Core/Context.h>
#include <Yngin/Core/Scenes.h>

namespace Yngin {
	ScriptsManager::ScriptsManager(Context* ctx) {
		impl = std::make_unique<Impl>();

		impl->ctx = ctx;
		impl->lua.open_libraries(sol::lib::base, sol::lib::math, sol::lib::table, sol::lib::string);
	}

	ScriptsManager::~ScriptsManager() = default;

	Context* ScriptsManager::getContext() {
		return impl->ctx;
	}

	void ScriptsManager::Impl::bind() {
		bindGlmTypes();
		bindYnginTypes();
		createYnginTable();
	}

	Script* ScriptsManager::createScript(const char* scriptData, uint32_t id, bool override) {
		return createScript(nullptr, scriptData, id, override);
	}

	Script* ScriptsManager::createScript(Scene* scene, const char* scriptData, uint32_t id, bool override) {
		if (id != -1) {
			if (getScript(id) != nullptr) {
				if (override) {
					deleteScript(id);
				} else {
					return nullptr;
				}
			}
		} else {
			id = impl->nextId;
		}

		Script* script = new Script(impl->ctx, scene);
		impl->nextId = std::max(impl->nextId, id + 1);
		script->impl->id = id;
		impl->scripts[id] = std::unique_ptr<Script>(script);

		script->impl->createScriptTable();

		script->execute(scriptData);

		if (impl->ctx->getStatus() == CONTEXT_STATUS::RUNNING) {
			sol::protected_function scriptReady = script->impl->env["onReady"];

			if (scriptReady.valid()) {
				auto res = scriptReady();

				if (!res.valid()) {
					// TODO: add error logging
				}
			}
		}

		return script;
	}

	void ScriptsManager::deleteScript(uint32_t id) {
		impl->scripts.erase(id);
	}

	void ScriptsManager::deleteScript(Script* script) {
		if (script->impl->ctx == impl->ctx) {
			deleteScript(script->impl->id);
		}
	}

	size_t ScriptsManager::getScriptsCount() const {
		return impl->scripts.size();
	}

	std::vector<Script*> ScriptsManager::getScripts() const {
		std::vector<Script*> scripts;
		for (auto& kvp : impl->scripts) {
			scripts.push_back(kvp.second.get());
		}
		return scripts;
	}

	Script* ScriptsManager::getScript(uint32_t id) const {
		auto it = impl->scripts.find(id);
		if (it == impl->scripts.end()) return nullptr;

		return it->second.get();
	}

	bool ScriptsManager::execute(const char* script) {
		try {
			impl->lua.script(script);
		} catch (sol::error) {
			// TODO: add error logging
			return false;
		}
		return true;
	}

	void ScriptsManager::Impl::onReady() {
		for (auto& [id, script] : scripts) {
			sol::protected_function f = script->impl->env["onReady"];

			if (!f.valid()) continue;

			auto res = f();

			if (!res.valid()) {
				// TODO: add error logging
			}
		}
	}

	void ScriptsManager::Impl::onSceneActive() {
		if (ctx->getStatus() != CONTEXT_STATUS::RUNNING) return;

		double delta = ctx->getDeltaTime();
		Scene* activeScene = ctx->getScenesManager()->getActive();

		for (auto& [id, script] : scripts) {
			if (script->impl->scene != activeScene || script->impl->scene == nullptr) continue;

			sol::protected_function f = script->impl->env["onSceneActive"];

			if (!f.valid()) continue;

			auto res = f(delta);

			if (!res.valid()) {
				// TODO: add error logging
			}
		}
	}

	void ScriptsManager::Impl::onSceneInactive() {
		if (ctx->getStatus() != CONTEXT_STATUS::RUNNING) return;

		double delta = ctx->getDeltaTime();
		Scene* activeScene = ctx->getScenesManager()->getActive();

		for (auto& [id, script] : scripts) {
			if (script->impl->scene != activeScene || script->impl->scene == nullptr) continue;

			sol::protected_function f = script->impl->env["onSceneInactive"];

			if (!f.valid()) continue;

			auto res = f(delta);

			if (!res.valid()) {
				// TODO: add error logging
			}
		}
	}

	void ScriptsManager::Impl::onUpdate() {
		if (ctx->getStatus() != CONTEXT_STATUS::RUNNING) return;

		double delta = ctx->getDeltaTime();
		Scene* activeScene = ctx->getScenesManager()->getActive();

		for (auto& [id, script] : scripts) {
			if (script->impl->scene != activeScene && script->impl->scene != nullptr) continue;

			sol::protected_function f = script->impl->env["onUpdate"];

			if (!f.valid()) continue;

			auto res = f(delta);

			if (!res.valid()) {
				// TODO: add error logging
			}
		}
	}

	Script::Script(Context* ctx, Scene* scene) {
		impl = std::make_unique<Impl>();

		impl->ctx = ctx;
		impl->scene = scene;

		ScriptsManager* scriptsManager = ctx->getScriptsManager();
		sol::state& lua = scriptsManager->impl->lua;
		impl->env = sol::environment(lua, sol::create, lua.globals());
	}

	Script::~Script() {
		impl->env.clear();
	}

	Context* Script::getContext() {
		return impl->ctx;
	}

	Scene* Script::getScene() {
		return impl->scene;
	}

	void Script::Impl::createScriptTable() {
		ScriptsManager* scriptsManager = ctx->getScriptsManager();
		sol::state& lua = scriptsManager->impl->lua;
		sol::table Script = lua.create_table("Script");
		Script["ID"] = id;
		Script["Scene"] = scene;
	}

	bool Script::execute(const char* script) {
		ScriptsManager* scriptsManager = impl->ctx->getScriptsManager();
		sol::state& lua = scriptsManager->impl->lua;
		try {
			lua.script(script, impl->env);
		} catch (sol::error) {
			// TODO: add error logging
			return false;
		}
		return true;
	}
}
