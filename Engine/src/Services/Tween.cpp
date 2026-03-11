#include <Yngin/Services/Tween.h>
#include <Yngin/Core/GameObject.h>
#include <Yngin/Rendering/Cameras.h>
#include <Yngin/Core/Scenes.h>
#include "Services_Internal.h"
#include "../Core/GameObject/GameObject_Internal.h"
#include "../Rendering/Cameras/Cameras_Internal.h"

#define PI 3.14159265358979323846f
#define HALF_PI 1.57079632679f

namespace Yngin {
	namespace Services {
		Tween::Tween(Context* ctx) : Service(ctx) {
			impl = std::make_unique<Impl>();
		};

		Tween::~Tween() = default;

		void Tween::onUpdate() {
			std::vector<int> toDelete;

			Scene* activeScene = Service::impl->ctx->getScenesManager()->getActive();

			if (activeScene == nullptr) {
				for (auto& kvp : impl->processes) {
					impl->processes.erase(kvp.second->id);
				}
				return;
			}

			GameObjectsManager* gameObjMgr = activeScene->getGameObjectsManager();
			CamerasManager* camerasMgr = activeScene->getCamerasManager();

			for (auto& kvp : impl->processes) {
				TweenProcess* p = kvp.second.get();

				if (gameObjMgr->getGameObject(p->linkedGameObjectId) == nullptr) {
					toDelete.push_back(p->id);
					continue;
				}

				if (camerasMgr->getCamera(p->linkedCameraId) == nullptr) {
					toDelete.push_back(p->id);
					continue;
				}

				double time = Service::impl->ctx->getFrameStartTime();
				if (p->lastPause != -1) time = p->lastPause;

				float progress = static_cast<float>((time - p->startTime - p->totalPauseTime) / p->duration);
				if (progress >= 1.0f) {
					progress = 1.0f;
					toDelete.push_back(p->id);
				}

				for (auto& v : p->values) {
					if (progress >= 1.0f) {
						if (v.value) *v.value = v.target;
						if (v.onUpdate) v.onUpdate(v.target);
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

					float newValue = v.initial + (v.target - v.initial) * eased;

					if (v.value) *v.value = newValue;
					if (v.onUpdate) v.onUpdate(newValue);
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
			process->values.push_back(TweenValues{
				.initial = start,
				.target = target,
				.onUpdate = onUpdate
				});
			process->startTime = Service::impl->ctx->getFrameStartTime();

			impl->processes[id] = std::move(process);

			return id;
		}

		int Tween::tweenPos(GameObject* obj, glm::vec3 target, const TweenSettings& settings) {
			assert(obj->impl->ctx == Service::impl->ctx);

			if (obj->impl->ctx != Service::impl->ctx) return 0;

			glm::vec3 pos = obj->getPosition();

			int id = impl->nextId++;

			std::unique_ptr<TweenProcess> process = std::make_unique<TweenProcess>();
			process->id = id;
			process->duration = settings.duration;
			process->function = settings.function;

			for (int i = 0; i < 3; i++) {
				process->values.push_back(TweenValues{
					.initial = pos[i],
					.target = target[i],
					.value = &obj->impl->pos[i],
					});
			}

			process->startTime = Service::impl->ctx->getFrameStartTime();
			process->linkedGameObjectId = obj->getId();

			impl->processes[id] = std::move(process);

			return id;
		}

		int Tween::tweenPos(Camera* obj, glm::vec3 target, const TweenSettings& settings) {
			assert(obj->impl->ctx == Service::impl->ctx);

			if (obj->impl->ctx != Service::impl->ctx) return 0;

			glm::vec3 pos = obj->getPosition();

			int id = impl->nextId++;

			std::unique_ptr<TweenProcess> process = std::make_unique<TweenProcess>();
			process->id = id;
			process->duration = settings.duration;
			process->function = settings.function;

			for (int i = 0; i < 3; i++) {
				process->values.push_back(TweenValues{
					.initial = pos[i],
					.target = target[i],
					.value = &obj->impl->pos[i],
					});
			}

			process->startTime = Service::impl->ctx->getFrameStartTime();
			process->linkedCameraId = obj->getId();

			impl->processes[id] = std::move(process);

			return id;
		}

		bool Tween::isPaused(int tweenId) {
			if (!isActive(tweenId)) return false;

			return impl->processes[tweenId]->lastPause != -1;
		}

		void Tween::setPaused(int tweenId, bool paused) {
			if (!isActive(tweenId)) return;

			TweenProcess* p = impl->processes[tweenId].get();
			if (paused && p->lastPause != -1) return;

			double time = Service::impl->ctx->getFrameStartTime();
			if (!paused) {
				p->totalPauseTime += time - p->lastPause;
				p->lastPause = -1;
				return;
			}

			p->lastPause = time;
		}
	}
}
