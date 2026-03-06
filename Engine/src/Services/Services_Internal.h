#pragma once
#include <Yngin/Services/Services.h>
#include <typeindex>
#include <any>

struct TweenValues {
	float initial;
	float target;
	float* value = nullptr;
	std::function<void(float)> onUpdate;
};

struct TweenProcess {
	int id = 0;

	std::vector<TweenValues> values;

	uint32_t linkedGameObjectId = 0;
	uint32_t linkedCameraId = 0;

	double startTime = 0;
	double duration = 1;

	double lastPause = -1;
	double totalPauseTime = 0;

	Yngin::Services::TWEEN_FUNCTION function;
};

namespace Yngin {
	namespace Services {
		struct Service::Impl {
			Context* ctx;
		};

		struct Tween::Impl {
			std::map<int, std::unique_ptr<TweenProcess>> processes;

			uint32_t nextId = 1;
		};
	}
}
