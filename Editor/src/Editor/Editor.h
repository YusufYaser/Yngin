#pragma once
#include <Yngin/Core/GameObject.h>
#include <Yngin/Core/Context.h>
#include <Yngin/Core/Scenes.h>
#include <Yngin/Rendering/Cameras.h>
#include <Yngin/Rendering/Textures.h>
#include <Yngin/Core/Models.h>
#include <Yngin/UI/Elements/Image.h>
#include <ImGui/imgui.h>
#include <sstream>
#include <ImGuiColorTextEdit/TextEditor.h>

enum class EXPLORER_SELECTION_TYPE {
	NONE,
	GAMEOBJECT,
	UIELEMENT,
	MODEL,
	TEXTURE,
	SCENE,
	GAME,
	SCRIPT
};

struct EditorScript {
	std::string name;
	uint32_t scene = uint32_t(-1);
	std::string code;
};

struct ScriptFileHeader {
	char magic[19];
	uint8_t version;
	uint32_t scriptsCount;
};

struct ScriptInfo {
	uint32_t id;
	uint32_t scene;
	size_t nameSize;
	char name[32];
	size_t scriptSize;
};

class Editor {
public:
	Editor();
	~Editor();

	void update();

	Yngin::Context* ctx;
	Yngin::Scene* activeScene;
	Yngin::Scene* viewerScene;
	Yngin::Camera* editorCamera;

	std::pair<EXPLORER_SELECTION_TYPE, int> explorerSelection = {};

	bool running = false;

	bool viewingObject = false;

	struct {
		std::string name = "Game";
		int windowWidth = 800;
		int windowHeight = 600;
		bool fullscreen = false;
	} gameSettings;

	std::string projectName = "Game";

	double lastSaved = 0;

private:
	TextEditor scriptEditor;

	void resetContext();
	void setupViewerScene();

	void saveProject();

	void exportGame();
	void togglePlayMode();

	void handleCameraMovement(Yngin::Camera* camera);

	void showGameProps();
	void showScriptProps(uint32_t id);
	void showGameObjectProps(uint32_t id);
	void showModelProps(uint32_t id);
	void showTextureProps(uint32_t id);
	void showSceneProps(uint32_t id);

	void showGameExplorer();
	void showSceneExplorer();
	void showResourceExplorer();

	Yngin::GameObject* viewerObject;
	Yngin::UI::Image* viewerImage;

	void setupPreviousGameState();
	void loadPreviousGameState();

	uint32_t nextScriptId = 0;
	std::map<uint32_t, EditorScript> scripts;

	void loadScripts();
};
