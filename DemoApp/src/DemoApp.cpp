#include <stdio.h>
#include <Yngin/Yngin.h>
#include <Yngin/Scenes.h>

int main() {
	Yngin::init();

	Yngin::Context* ctx = Yngin::createContext();

	printf("Context: %p\n", ctx);

	Yngin::Scene* scene = Yngin::createScene(ctx);

	while (!ctx->windowShouldClose()) {
		ctx->updateWindow();
		scene->render();
		ctx->swapBuffers();
	}

	Yngin::terminate();

	return 0;
}
