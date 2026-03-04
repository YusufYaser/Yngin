#include <Yngin/Services/Tween.h>
#include <Yngin/Core/GameObject.h>
#include "Services_Internal.h"
#include "../Core/GameObject/GameObject_Internal.h"

#define PI 3.14159265358979323846
#define HALF_PI 1.57079632679f

namespace Yngin {
	namespace Services {
		Tween::Tween(Context* ctx) : Service(ctx) {
			impl = std::make_unique<Impl>();
		};

		Tween::~Tween() = default;

		void Tween::onUpdate() {
			double time = Service::impl->ctx->getTime();

			std::vector<int> toDelete;

			for (auto& kvp : impl->processes) {
				for (auto& ptr : kvp.second) {
					TweenProcess* p = ptr.get();

					float progress = (time - p->startTime) / p->duration;
					if (progress >= 1.0f) {
						progress = 1.0f;
						toDelete.push_back(p->id);
						if (p->value) *p->value = p->target;
						if (p->onUpdate) p->onUpdate(p->target);
						continue;
					}

					float eased = 0;

					switch (p->function) {
					case TWEEN_FUNCTION::LINEAR:
						eased = progress;
						break;
					case TWEEN_FUNCTION::EASE_IN:
						eased = 1 - cos(progress * HALF_PI);
						break;
					case TWEEN_FUNCTION::EASE_OUT:
						eased = sin(progress * HALF_PI);
						break;
					case TWEEN_FUNCTION::EASE_INOUT:
						eased = -(cos(progress * PI) - 1) / 2;
						break;
					}

					float newValue = p->initial + (p->target - p->initial) * eased;

					if (p->value) *p->value = newValue;
					if (p->onUpdate) p->onUpdate(newValue);
				}
			}

			for (int id : toDelete) {
				impl->processes.erase(id);
			}
		}

		void Tween::cancel(int tweenId) {
			impl->processes.erase(tweenId);
		}

		bool Tween::isActive(int tweenId) {
			return impl->processes.find(tweenId) != impl->processes.end();
		}

		int Tween::tweenFloat(float start, float target, const TweenSettings& settings, std::function<void(float)> onUpdate) {
			int id = impl->nextId++;

			std::unique_ptr<TweenProcess> process = std::make_unique<TweenProcess>();
			process->id = id;
			process->duration = settings.duration;
			process->function = settings.function;
			process->initial = start;
			process->target = target;
			process->value = nullptr;
			process->onUpdate = onUpdate;
			process->startTime = Service::impl->ctx->getTime();

			impl->processes[id].push_back(std::move(process));

			return id;
		}

		int Tween::tweenPos(GameObject* obj, glm::vec3 target, const TweenSettings& settings) {
			assert(obj->impl->ctx == Service::impl->ctx);

			if (obj->impl->ctx != Service::impl->ctx) return 0;

			glm::vec3 pos = obj->getPos();

			int id = impl->nextId++;

			for (int i = 0; i < 3; i++) {
				std::unique_ptr<TweenProcess> process = std::make_unique<TweenProcess>();
				process->id = id;
				process->duration = settings.duration;
				process->function = settings.function;
				process->initial = pos[i];
				process->target = target[i];
				// I know this is unsafe, I'll fix this soon
				process->value = &obj->impl->pos[i];
				process->startTime = Service::impl->ctx->getTime();

				impl->processes[id].push_back(std::move(process));
			}

			return id;
		}
	}
}
