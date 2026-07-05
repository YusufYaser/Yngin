#pragma once
#include "../EditorWindow.h"

class ScriptOutputWindow : public EditorWindow {
public:
	ScriptOutputWindow(Editor* editor) : EditorWindow(editor) {}
	~ScriptOutputWindow() = default;

	void draw() override;

private:
	int logsStart = 0;
	std::string filter = "";
	std::string execute = "";
};