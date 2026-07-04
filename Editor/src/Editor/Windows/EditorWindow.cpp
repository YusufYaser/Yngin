#include "EditorWindow.h"

EditorWindow::EditorWindow(Editor* editor) {
	uid = editor->nextWindowUid++;

	this->editor = editor;
}

EditorWindow::~EditorWindow() = default;

bool EditorWindow::shouldClose() const {
	return !isWindowOpen;
}

uint32_t EditorWindow::getUid() const {
	return uid;
}

std::string EditorWindow::getWindowImGuiId() const {
	return "Window" + std::to_string(uid);
}

std::string EditorWindow::getGuiId(const char* name) const {
	return std::string(name) + "##Window" + std::to_string(uid);
}
