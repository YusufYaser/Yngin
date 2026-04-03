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
	std::string code;
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

private:
	TextEditor scriptEditor;

	void resetContext();
	void setupViewerScene();

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

	Yngin::Texture* gridTexture;
	Yngin::Model* cubeModel;

	void setupPreviousGameState();
	void loadPreviousGameState();

	uint32_t nextScriptId = 0;
	std::map<uint32_t, EditorScript> scripts;

	void loadScripts();
};
