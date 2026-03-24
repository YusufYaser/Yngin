#pragma once
#include <Yngin/Forward.h>
#include "Service.h"
#include <memory>
#include <glm/vec3.hpp>

namespace Yngin {
	namespace Services {
		enum class TWEEN_FUNCTION : uint8_t {
			LINEAR = 0,
			EASE_IN,
			EASE_OUT,
			EASE_INOUT,
		};

		struct TweenSettings {
			double duration;
			TWEEN_FUNCTION function;
		};

		class Tween : public Service {
		public:
			void cancel(int tweenId);

			bool isActive(int tweenId);

			int tweenFloat(float start, float target, const TweenSettings& settings, std::function<void(float)> onUpdate);
			int tweenPos(GameObject* obj, glm::vec3 target, const TweenSettings& settings);
			int tweenPos(Camera* obj, glm::vec3 target, const TweenSettings& settings);

			bool isPaused(int tweenId);
			void setPaused(int tweenId, bool paused);

		private:
			friend class Context;
			friend struct std::default_delete<Tween>;
			friend class Scene;

			Tween(Context* ctx);
			~Tween();

			void onUpdate();

			struct Impl;
			std::unique_ptr<Impl> impl;
		};
	}
}
