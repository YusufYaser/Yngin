#pragma once
#include <ImGui/imgui.h>
#include <GLFW/glfw3.h>

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
};
