#include <Yngin/Core/Scripting.h>
#include <sol/sol.hpp>
#include "Scripting_Internal.h"
#include <Yngin/Core/Context.h>
#include <Yngin/Core/Scenes.h>
#include <stdio.h>

namespace Yngin {
	ScriptsManager::ScriptsManager(Context* ctx) {
		impl = std::make_unique<Impl>();

		impl->ctx = ctx;
		impl->lua.open_libraries(sol::lib::base, sol::lib::math, sol::lib::table, sol::lib::string, sol::lib::os);
	}

	ScriptsManager::~ScriptsManager() = default;

	Context* ScriptsManager::getContext() const {
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

		if (scriptData != "") {
			sol::load_result chunk = impl->lua.load(scriptData);

			if (chunk.valid()) {
				sol::protected_function func = chunk;

				lua_State* L = impl->lua.lua_state();
				func.push(L);

				std::vector<char> byteCode;

				lua_dump(L, [](lua_State*, const void* p, size_t size, void* out) -> int {
					std::vector<char>* bc = static_cast<std::vector<char>*>(out);
					const char* data = static_cast<const char*>(p);
					bc->insert(bc->end(), data, data + size);
					return 0;
					}, &byteCode, 1);

				script->impl->byteCode = byteCode;

				sol::set_environment(script->impl->env, func);

				if (impl->ctx->meta.getMetaInt("#IsEditor", 0) != 1 || impl->ctx->meta.getMetaInt("#IsPlaying", 0) == 1) {
					sol::protected_function_result res = func();

					if (!res.valid()) {
						sol::error error = res;

						printf("[Yngin] [Script #%i] Error while loading script: %s\n", id, error.what());
					}
				}
			} else {
				sol::error error = chunk;

				printf("[Yngin] [Script #%i] Error while loading script: %s\n", id, error.what());
			}
		}

		if (impl->ctx->getStatus() == CONTEXT_STATUS::RUNNING) {
			sol::protected_function scriptReady = script->impl->env["onReady"];

			if (scriptReady.valid()) {
				auto res = scriptReady();

				if (!res.valid()) {
					sol::error error = res;

					printf("[Yngin] [Script #%i] Error while invoking onReady(): %s\n", id, error.what());
				}
			}
		}

		return script;
	}

	void ScriptsManager::deleteScript(uint32_t id) {
		if (impl->deleteQueueEnabled) {
			impl->deleteQueue.push_back(id);
			return;
		}
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
		bool success = true;

		impl->deleteQueueEnabled = true;
		try {
			impl->lua.script(script);
		} catch (sol::error error) {
			printf("[Yngin] [ScriptsManager] Error while executing code globally: %s\n", error.what());
			success = false;
		}
		impl->deleteQueueEnabled = false;

		for (auto& id : impl->deleteQueue) {
			impl->ctx->getScriptsManager()->deleteScript(id);
		}

		impl->deleteQueue.clear();
		return success;
	}

	void ScriptsManager::Impl::onReady() {
		deleteQueueEnabled = true;
		for (auto& [id, script] : scripts) {
			if (!script->impl->enabled) continue;
			sol::protected_function f = script->impl->env["onReady"];

			if (!f.valid()) continue;

			auto res = f();

			if (!res.valid()) {
				sol::error error = res;

				printf("[Yngin] [Script #%i] Error while invoking onReady(): %s\n", id, error.what());
			}
		}
		deleteQueueEnabled = false;

		for (auto& id : deleteQueue) {
			ctx->getScriptsManager()->deleteScript(id);
		}

		deleteQueue.clear();
	}

	void ScriptsManager::Impl::onSceneActive() {
		if (ctx->getStatus() != CONTEXT_STATUS::RUNNING) return;

		double delta = ctx->getDeltaTime();
		Scene* activeScene = ctx->getScenesManager()->getActive();

		deleteQueueEnabled = true;
		for (auto& [id, script] : scripts) {
			if (!script->impl->enabled) continue;
			if (script->impl->scene != activeScene || script->impl->scene == nullptr) continue;

			sol::protected_function f = script->impl->env["onSceneActive"];

			if (!f.valid()) continue;

			auto res = f(delta);

			if (!res.valid()) {
				sol::error error = res;

				printf("[Yngin] [Script #%i] Error while invoking onSceneActive(): %s\n", id, error.what());
			}
		}
		deleteQueueEnabled = false;

		for (auto& id : deleteQueue) {
			ctx->getScriptsManager()->deleteScript(id);
		}

		deleteQueue.clear();
	}

	void ScriptsManager::Impl::onSceneInactive() {
		if (ctx->getStatus() != CONTEXT_STATUS::RUNNING) return;

		double delta = ctx->getDeltaTime();
		Scene* activeScene = ctx->getScenesManager()->getActive();

		deleteQueueEnabled = true;
		for (auto& [id, script] : scripts) {
			if (!script->impl->enabled) continue;
			if (script->impl->scene != activeScene || script->impl->scene == nullptr) continue;

			sol::protected_function f = script->impl->env["onSceneInactive"];

			if (!f.valid()) continue;

			auto res = f(delta);

			if (!res.valid()) {
				sol::error error = res;

				printf("[Yngin] [Script #%i] Error while invoking onSceneInactive(): %s\n", id, error.what());
			}
		}
		deleteQueueEnabled = false;

		for (auto& id : deleteQueue) {
			ctx->getScriptsManager()->deleteScript(id);
		}

		deleteQueue.clear();
	}

	void ScriptsManager::Impl::onUpdate() {
		if (ctx->getStatus() != CONTEXT_STATUS::RUNNING) return;

		double delta = ctx->getDeltaTime();
		Scene* activeScene = ctx->getScenesManager()->getActive();

		deleteQueueEnabled = true;
		for (auto& [id, script] : scripts) {
			if (!script->impl->enabled) continue;
			if (script->impl->scene != activeScene && script->impl->scene != nullptr) continue;

			sol::protected_function f = script->impl->env["onUpdate"];

			if (!f.valid()) continue;

			auto res = f(delta);

			if (!res.valid()) {
				sol::error error = res;

				printf("[Yngin] [Script #%i] Error while invoking onUpdate(): %s\n", id, error.what());
			}
		}
		deleteQueueEnabled = false;

		for (auto& id : deleteQueue) {
			ctx->getScriptsManager()->deleteScript(id);
		}

		deleteQueue.clear();
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

	uint32_t Script::getId() {
		return impl->id;
	}

	bool Script::isEnabled() const {
		return impl->enabled;
	}

	void Script::setEnabled(bool enabled) {
		impl->enabled = enabled;
	}

	void Script::Impl::createScriptTable() {
		ScriptsManager* scriptsManager = ctx->getScriptsManager();
		sol::state& lua = scriptsManager->impl->lua;
		sol::table Script = lua.create_table("Script");
		Script["ID"] = id;
		Script["Scene"] = scene;
	}

	bool Script::execute(const char* script) {
		if (!impl->enabled) return false;

		ScriptsManager* scriptsManager = impl->ctx->getScriptsManager();
		sol::state& lua = scriptsManager->impl->lua;
		try {
			lua.script(script, impl->env);
		} catch (sol::error error) {
			printf("[Yngin] [Script #%i] Error while executing code: %s\n", impl->id, error.what());
			return false;
		}
		return true;
	}
}
