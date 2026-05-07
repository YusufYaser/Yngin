#pragma once
#include <string>
#include <stdint.h>

class Editor;

class EditorUI {
public:
	EditorUI(Editor* editor) : editor(editor) {}

	Editor* editor;

	bool textureSelector(std::string id, uint32_t* v);
	bool modelSelector(std::string id, uint32_t* v);
	bool materialSelector(std::string id, uint32_t* v);
};
