#pragma once
#include <iostream>
#include <map>

namespace Yngin {
	class ModelsManager;
	class TexturesManager;
	class ScriptsManager;
	class GameObjectsManager;
	class CamerasManager;
	class Scene;

	namespace UI {
		class UIManager;
	}


	namespace GameFiles {
		namespace {
			std::map<int, int> dummyMap;
		}

		enum OP : uint8_t {
			MODEL,
			TEXTURE,
			SCRIPT,
			SCENE,
			GAMEOBJECT,
			COMPONENT,
			CAMERA,
			UIELEMENT
		};

		struct Operation {
			OP op;
		};

		class Generators {
		public:
			static bool modelsManager(std::ostream& s, const ModelsManager* mgr);
			static bool texturesManager(std::ostream& s, TexturesManager* mgr);
			static bool scriptsManager(std::ostream& s, const ScriptsManager* mgr, Scene* scene = nullptr);
			static bool gameObjectsManager(std::ostream& s, const GameObjectsManager* mgr);
			static bool camerasManager(std::ostream& s, const CamerasManager* mgr);
			static bool uiManager(std::ostream& s, const UI::UIManager* mgr);
		};

		class Loaders {
		public:
			static bool modelsManager(std::istream& s, ModelsManager* mgr);
			static bool texturesManager(std::istream& s, TexturesManager* mgr);
			static bool scriptsManager(std::istream& s, ScriptsManager* mgr);

			static bool camerasManager(std::istream& s, CamerasManager* mgr);
			static bool uiManager(std::istream& s, UI::UIManager* mgr, std::map<int, int>& parentsQueue = dummyMap);
		};
	}
}
