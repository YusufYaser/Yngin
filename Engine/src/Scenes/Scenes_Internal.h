#pragma once
#include <Yngin/Yngin.h>

namespace Yngin {
	class CamerasManager;

	struct Scene::Impl {
		Context* ctx;
		std::unique_ptr<CamerasManager> camerasManager;
	};
}