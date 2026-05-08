#pragma once
#include <ImGui/imgui.h>
#include <GLFW/glfw3.h>
#include <map>
#include <string>

struct RecentProject {
	std::string name;
	std::string path;
	uint64_t lastOpened;
};

class StartWindow {
public:
	StartWindow();
	~StartWindow();

	void update();

	bool shouldClose() const;

	GLFWwindow* window;

private:
	ImGuiContext* imguiCtx;

	bool closing = false;

	ImFont* defaultFont;
	ImFont* titleFont;

	std::map<std::string, RecentProject> recentProjects;
};
