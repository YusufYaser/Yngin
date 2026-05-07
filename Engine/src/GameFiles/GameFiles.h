#pragma once
#include <Yngin/Forward.h>
#include <iostream>
#include <map>

#define ValidatorCheck(type, count) if (!( \
	s.good() && \
	(count) >= size_t(0) && \
	(count) < (SIZE_MAX / sizeof(type)) && \
	s.rdbuf()->in_avail() >= (count) * sizeof(type) \
)) return false;

#define Seek(count) s.seekg(count, std::ios::cur)

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
			UIELEMENT,
			GAMEDATA,
			MATERIAL
		};

		enum META_TYPE : uint8_t {
			STRING,
			INT,
			FLOAT,
			POINTER // void pointers cannot be exported but they are still included here
		};

		struct Operation {
			OP op;
		};

		struct MetaHeader {
			size_t metasCount;
		};

		// meta tags must be generated and loaded in the exact order in the header

		struct MetaGeneric {
			META_TYPE type;
			size_t keyLength;
			// char key[keyLength];
			// .. data
		};

		struct MetaStringData {
			size_t length;
			// char str[length];
		};

		class Generators {
		public:
			static bool modelsManager(std::ostream& s, const ModelsManager* mgr);
			static bool texturesManager(std::ostream& s, TexturesManager* mgr, bool compressed = false);
			static bool scriptsManager(std::ostream& s, const ScriptsManager* mgr, Scene* scene = nullptr);
			static bool gameObjectsManager(std::ostream& s, const GameObjectsManager* mgr);
			static bool camerasManager(std::ostream& s, const CamerasManager* mgr);
			static bool uiManager(std::ostream& s, const UI::UIManager* mgr);
			static bool materialsManager(std::ostream& s, const MaterialsManager* mgr);
			static bool meta(std::ostream& s, const Meta& meta, Context* ctx);
		};

		class Loaders {
		public:
			static bool modelsManager(std::istream& s, ModelsManager* mgr);
			static bool texturesManager(std::istream& s, TexturesManager* mgr);
			static bool scriptsManager(std::istream& s, ScriptsManager* mgr);

			static bool camerasManager(std::istream& s, CamerasManager* mgr);
			static bool uiManager(std::istream& s, UI::UIManager* mgr, std::map<int, int>& parentsQueue = dummyMap);
			static bool materialsManager(std::istream& s, MaterialsManager* mgr);
			static bool meta(std::istream& s, Meta& meta, Context* ctx);
		};

		class Validators {
		public:
			static bool modelsManager(std::istream& s);
			static bool texturesManager(std::istream& s);
			static bool scriptsManager(std::istream& s);

			static bool camerasManager(std::istream& s);
			static bool uiManager(std::istream& s);
			static bool materialsManager(std::istream& s);
			static bool meta(std::istream& s);
		};
	}
}
