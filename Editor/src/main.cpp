#include "Editor/Editor.h"
#include <ImGui/imgui.h>
#include <Yngin/Core/Context.h>


int main() {
	IMGUI_CHECKVERSION();

	Yngin::initializeYngin();

	if (!Yngin::isYnginInitialized()) {
		printf("Failed to initialize Yngin\n");
		return 1;
	}

	Editor* editor = new Editor();
	while (editor->ctx->getStatus() == Yngin::CONTEXT_STATUS::RUNNING) editor->update();

	delete editor;
	editor = nullptr;

	Yngin::terminateYngin();

	return 0;
}
