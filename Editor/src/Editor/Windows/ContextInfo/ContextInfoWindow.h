#pragma once
#include "../EditorWindow.h"

class ContextInfoWindow : public EditorWindow {
public:
	ContextInfoWindow(Editor* editor) : EditorWindow(editor) {};
	~ContextInfoWindow() = default;

	void draw() override;
};
