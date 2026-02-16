#pragma once
#include <Yngin/Yngin.h>

namespace Yngin {
	class CamerasManager;

	class Scene {
	public:
		Scene(Context* ctx);
		~Scene();

		Context* getContext() { return ctx; };

		CamerasManager* getCamerasManager();

		void render();

	private:
		Context* ctx;
		std::unique_ptr<CamerasManager> camerasManager;
	};
}