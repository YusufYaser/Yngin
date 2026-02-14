#include <stdio.h>
#include <Yngin/Yngin.h>

int main() {
	Yngin::init();

	Yngin::Context* ctx = Yngin::createContext();

	printf("Context: %p\n", ctx);

	while (!ctx->windowShouldClose()) ctx->updateWindow();

	Yngin::terminate();

	return 0;
}
