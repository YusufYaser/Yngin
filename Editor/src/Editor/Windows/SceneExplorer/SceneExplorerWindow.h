#pragma once
#include "../EditorWindow.h"

class SceneExplorerWindow : public EditorWindow {
public:
	SceneExplorerWindow(Editor* editor) : EditorWindow(editor) {}
	~SceneExplorerWindow() = default;

	void draw() override;

private:
	void showGameExplorer();
	void showSceneExplorer();
	void showResourceExplorer();
};
