#pragma once
#include "../EditorWindow.h"

class PropertiesWindow : public EditorWindow {
public:
	PropertiesWindow(Editor* editor) : EditorWindow(editor) {}
	~PropertiesWindow() = default;

	void draw() override;

private:
	void showGameProps();
	void showScriptProps(uint32_t id);
	void showGameObjectProps(uint32_t id);
	void showUIElementProps(uint32_t id, bool global = false);
	void showModelProps(uint32_t id);
	void showMaterialProps(uint32_t id);
	void showTextureProps(uint32_t id);
	void showSceneProps(uint32_t id);

	std::string path = "";
	glm::vec3 newModelOffset = {};
	glm::vec3 newModelRotation = {};
	glm::vec3 newModelScale = {};
};
