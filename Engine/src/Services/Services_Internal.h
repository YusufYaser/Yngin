#pragma once
#include <Yngin/Services/Services.h>
#include <typeindex>
#include <any>

struct TweenProcess {
	int id;

	float* value;
	std::function<void(float)> onUpdate;

	double startTime;
	double duration = 1;

	float initial;
	float target;

	Yngin::Services::TWEEN_FUNCTION function;
};

namespace Yngin {
	namespace Services {
		struct Service::Impl {
			Context* ctx;
		};

		struct Tween::Impl {
			std::map<int, std::vector<std::unique_ptr<TweenProcess>>> processes;

			uint32_t nextId = 1;
		};
	}
}
