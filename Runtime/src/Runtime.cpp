#include <stdio.h>
#include <Yngin/Core/Context.h>
#include <Yngin/Core/Scenes.h>
#include <Yngin/Core/InputSystem.h>
#include <Yngin/Rendering/Cameras.h>
#include <Yngin/Services/SerializationService.h>
#include <fstream>
#include <sstream>
#include <format>

#ifdef _WIN32
#include <windows.h>
#endif

using namespace Yngin;

void error(const char* text) {
	printf("[Yngin Runtime] %s\n", text);
#ifdef _WIN32
	MessageBoxA(NULL, text, "Yngin Runtime", MB_ICONERROR | MB_OK);
#endif
}

int main() {
#ifdef _WIN32
	bool consoleShown = true;

	if (AllocConsole()) {
		FILE* fDummy;
		freopen_s(&fDummy, "CONOUT$", "w", stdout);
		freopen_s(&fDummy, "CONOUT$", "w", stderr);
		freopen_s(&fDummy, "CONIN$", "r", stdin);
	}
#ifndef _DEBUG
	ShowWindow(GetConsoleWindow(), SW_HIDE);
	consoleShown = false;
#endif
#endif

	Yngin::initializeYngin();

	if (!Yngin::isYnginInitialized()) {
		error("Failed to initialize Yngin");
		return 1;
	}

	std::unique_ptr<Context> ctx(Context::createContext());

	if (!ctx || ctx->getStatus() != CONTEXT_STATUS::WAITING_FOR_READY) {
		error("Failed to create a Yngin context");

		ctx.reset();

		Yngin::terminateYngin();
		return 1;
	}

	Services::SerializationService* serializationService = ctx->getService<Services::SerializationService>();

	std::ifstream manifestFile("manifest.bin", std::ios::binary);
	if (!manifestFile) {
		error("Failed to open manifest.bin");
		ctx.reset();
		Yngin::terminateYngin();
		return 1;
	}

	Meta manifest{};

	{
		Services::DeserializationContext deserializationContext{};
		deserializationContext.targetMeta = &manifest;
		if (serializationService->load(manifestFile, deserializationContext) != Services::DESERIALIZATION_STATUS::OK) {
			error("Corrupted game manifest");
			ctx.reset();
			Yngin::terminateYngin();
			return 1;
		}
	}

	glm::ivec2 windowSize = { manifest.getMetaInt("windowWidth", 800), manifest.getMetaInt("windowHeight", 600) };
	std::string windowTitle = manifest.getMetaString("name", "Yngin Game");
	bool fullscreen = manifest.getMetaInt("fullscreen", 0) != 0;
	std::vector<int> scenes = manifest.getMetaIntArray("scenes", {});

	Window* window = ctx->getWindow();
	window->setTitle(windowTitle.c_str());
	window->setSize(windowSize);
	window->setFullscreen(fullscreen);
	window->setPositionCentered();

	if (scenes.size() == 0) {
		error("Invalid game manifest data");
		ctx.reset();
		Yngin::terminateYngin();
		return 1;
	}

	for (auto& sceneId : scenes) {
		std::string fileName = std::format("scene{}.bin", sceneId);
		std::ifstream sceneFile(fileName, std::ios::binary);
		if (!sceneFile) {
			error(std::format("Failed to open scene file: {}", fileName).c_str());
			ctx.reset();
			Yngin::terminateYngin();
			return 1;
		}

		Meta sceneInfo{};

		Scene* scene = ctx->getScenesManager()->createScene(sceneId, true);
		Services::DeserializationContext deserializationContext{};
		deserializationContext.overrideConflictingId = true;
		deserializationContext.targetScene = scene;
		deserializationContext.targetMeta = &sceneInfo;
		deserializationContext.targetUIManager = scene->getUIManager();
		if (serializationService->load(sceneFile, deserializationContext) != Services::DESERIALIZATION_STATUS::OK) {
			error(std::format("Failed to load scene file: {}", fileName).c_str());
			ctx.reset();
			Yngin::terminateYngin();
			return 1;
		}

		scene->setGravity(sceneInfo.getMetaFloat("gravity", 9.8f));
		scene->setSkyboxTexture(sceneInfo.getMetaInt("skyboxTextureId", 0));
	}

	uint32_t defaultSceneId = manifest.getMetaInt("defaultScene", -1);
	ctx->getScenesManager()->setActive(defaultSceneId);
	if (ctx->getScenesManager()->getActive() == nullptr) {
		error("Invalid game manifest data");
		ctx.reset();
		Yngin::terminateYngin();
		return 1;
	}

	{
		auto resourcesFile = std::make_unique<std::ifstream>("resources.pak", std::ios::binary);
		if (!resourcesFile || !resourcesFile.get()) {
			error("Failed to open resources.pak");
			ctx.reset();
			Yngin::terminateYngin();
			return 1;
		}

		std::unique_ptr<std::istream> stream = std::move(resourcesFile);

		if (serializationService->streamArchive(stream, {}) == 0) {
			error("Corrupted game resources");
			ctx.reset();
			Yngin::terminateYngin();
			return 1;
		}
	}

	InputSystem* input = ctx->getInputSystem();

	ctx->ready();
	while (ctx->getStatus() == CONTEXT_STATUS::RUNNING) {
#ifdef _WIN32
		if (input->isKeyJustPressed(KEY::F4)) {
			consoleShown = !consoleShown;
			bool wasFocused = window->isFocused();
			ShowWindow(GetConsoleWindow(), consoleShown ? SW_SHOW : SW_HIDE);
			if (wasFocused) window->setFocused();
		}
#endif

		ctx->update();
	}

	ctx.reset();

	Yngin::terminateYngin();

	return 0;
}

#ifdef _WIN32
int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR pCmdLine, int nCmdShow) {
	return main();
}
#endif
