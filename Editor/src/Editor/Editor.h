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
#include <ImPlot/implot.h>
#include "UI/UI.h"
#ifdef _WIN32
#include <Windows.h>
#undef min
#undef max
#endif

enum class EXPLORER_SELECTION_TYPE {
	NONE,
	GAMEOBJECT,
	UIELEMENT,
	MODEL,
	MATERIAL,
	TEXTURE,
	SCENE,
	GAME,
	SCRIPT
};

struct EditorScript {
	std::string name;
	uint32_t scene = uint32_t(-1);
	std::string code;
	TextEditor editor;
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

class EditorUI;
class EditorWindow;

class Editor {
public:
	Editor(std::string path);
	~Editor();

	static bool generateNewProject(std::string path);

	std::string path;

	void update();

	std::unique_ptr<EditorUI> ui;

	Yngin::Context* ctx;
	Yngin::Scene* activeScene;
	Yngin::Scene* viewerScene;
	Yngin::Camera* editorCamera;

	std::pair<EXPLORER_SELECTION_TYPE, int> explorerSelection = {};

	bool running = false;
	double runningStartTime = 0;

	bool editorLighting = true;

	bool viewingObject = false;

	struct {
		std::string name = "Game";
		int windowWidth = 800;
		int windowHeight = 600;
		bool fullscreen = false;
	} gameSettings;

	std::string projectName = "Game";

	double lastSaved = 0;

	uint32_t nextScriptId = 0;
	std::map<uint32_t, EditorScript> scripts;

	//private:
	ImGuiContext* imguiCtx;
	ImPlotContext* implotCtx;

	bool filesLoaded = false;

#ifdef _WIN32
	HANDLE mutex;
#endif

	uint32_t nextWindowUid = 0;
	std::vector<std::unique_ptr<EditorWindow>> windows;

	void resetContext();
	void setupViewerScene();

	void saveProject();
	static bool saveContext(Yngin::Context* ctx, std::map<uint32_t, EditorScript> scripts = {});

	void exportGame();
	void togglePlayMode();

	std::pair<EXPLORER_SELECTION_TYPE, int> explorerSelectionBeforePlaying = {};

	void handleCameraMovement(Yngin::Camera* camera);

	void showGameExplorer();
	void showSceneExplorer();
	void showResourceExplorer();

	Yngin::GameObject* viewerObject;
	Yngin::UI::Image* viewerImage;
	Yngin::GameObject* viewerLightObject;

	void setupPreviousGameState();
	void loadPreviousGameState();

	void loadScripts();

	int logsStart = 0;

	float lastGraphTime = -5.0f;
	std::vector<float> graphsTimes;
	std::vector<float> graphsFPSValues;
	std::vector<float> graphsMemoryValues;
};
