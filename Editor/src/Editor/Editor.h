#pragma once
#include <Yngin/Core/GameObject.h>
#include <Yngin/Core/Context.h>
#include <Yngin/Core/Scenes.h>
#include <Yngin/Rendering/Cameras.h>
#include <ImGui/imgui.h>

enum class EXPLORER_SELECTION_TYPE {
	NONE,
	GAMEOBJECT,
	UIELEMENT
};

class Editor {
public:
	Editor();
	~Editor();

	void update();

	Yngin::Context* ctx;
	Yngin::Scene* activeScene;
	Yngin::Camera* editorCamera;

	std::pair<EXPLORER_SELECTION_TYPE, int> explorerSelection = {};

	bool simulating = false;

private:
	void handleCameraMovement(Yngin::Camera* camera);
	void gameObjectProps(Yngin::GameObject* obj);
	void showExplorer();
};
