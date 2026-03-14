#pragma once
#include <Yngin/Core/GameObject.h>
#include <Yngin/Core/Context.h>
#include <Yngin/Core/Scenes.h>
#include <Yngin/Rendering/Cameras.h>
#include <Yngin/Rendering/Textures.h>
#include <Yngin/Core/Models.h>
#include <ImGui/imgui.h>

enum class EXPLORER_SELECTION_TYPE {
	NONE,
	GAMEOBJECT,
	UIELEMENT,
	MODEL,
	TEXTURE
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

	bool simulating = false;

	bool viewingObject = false;

private:
	void handleCameraMovement(Yngin::Camera* camera);
	void gameObjectProps(uint32_t id);
	void showSceneExplorer();
	void showResourceExplorer();

	Yngin::GameObject* viewerObject;

	Yngin::Texture* gridTexture;
	Yngin::Model* squareModel;
};
