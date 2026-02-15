#pragma once
#include <Yngin/Yngin.h>

namespace Yngin {
	class Scene {
	public:
		Scene(Context* ctx) : ctx(ctx) {};
		~Scene();

		Context* getContext() { return ctx; };

		void render();

	private:
		Context* ctx;
	};

	Scene* createScene(Context* ctx);
}
