#pragma once
#include <Yngin/Forward.h>
#include <memory>
#include <vector>
#include <Yngin/Utils/Meta.h>

namespace Yngin {
	namespace GameFiles {
		class Generators;
		class Loaders;
	}

	class Script {
	public:
		Context* getContext();
		Scene* getScene();
		uint32_t getId();

		Meta meta;

		bool isEnabled() const;
		void setEnabled(bool enabled);

		// execute code within the script environments
		bool execute(const char* script);

		std::vector<std::string> getScriptOutput() const;

	private:
		friend class ScriptsManager;
		friend struct std::default_delete<Script>;
		friend class Context;
		friend class GameFiles::Generators;
		friend class GameFiles::Loaders;

		Script(Context* ctx, Scene* scene);
		~Script();

		struct Impl;
		std::unique_ptr<Impl> impl;
	};

	class ScriptsManager {
	public:
		Context* getContext() const;

		void openUnsafeLibraries();

		Script* createScript(const char* script = "", uint32_t id = -1, bool override = false);
		Script* createScript(Scene* scene, const char* script = "", uint32_t id = -1, bool override = false);
		void deleteScript(uint32_t id);
		void deleteScript(Script* script);

		size_t getScriptsCount() const;
		std::vector<Script*> getScripts() const;

		Script* getScript(uint32_t id) const;

		std::vector<std::pair<uint32_t, std::string>> getGlobalOutput() const;

		// execute code in the global context
		bool execute(const char* script);

	private:
		friend class Context;
		friend struct std::default_delete<ScriptsManager>;
		friend class Script;
		friend class ScenesManager;
		friend class GameFiles::Loaders;

		ScriptsManager(Context* ctx);
		~ScriptsManager();

		struct Impl;
		std::unique_ptr<Impl> impl;
	};
}
