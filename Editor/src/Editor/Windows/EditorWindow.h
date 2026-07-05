#pragma once
#include "../Editor.h"
#include <Yngin/Forward.h>
#include <glm/glm.hpp>

class EditorWindow {
public:
	EditorWindow(Editor* editor);
	~EditorWindow();

	bool shouldClose() const;

	virtual void draw() = 0;

	uint32_t getUid() const;
	std::string getWindowImGuiId() const;

protected:
	uint32_t uid;
	Editor* editor;

	bool isWindowOpen = true;
};
