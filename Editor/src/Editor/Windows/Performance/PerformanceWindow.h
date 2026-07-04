#pragma once
#include "../EditorWindow.h"

class PerformanceWindow : public EditorWindow {
public:
	PerformanceWindow(Editor* editor) : EditorWindow(editor) {}
	~PerformanceWindow() = default;

	void draw() override;
};
