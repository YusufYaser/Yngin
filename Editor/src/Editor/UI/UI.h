#pragma once
#include <string>
#include <stdint.h>
#include <map>

class Editor;

class EditorUI {
public:
	EditorUI(Editor* editor) : editor(editor) {}

	Editor* editor;

	bool textureSelector(std::string id, uint32_t* v);
	bool modelSelector(std::string id, uint32_t* v);
	bool materialSelector(std::string id, uint32_t* v);

	bool fileSelector(std::string name, std::map<std::string, std::string> filters, std::string* path);
};
