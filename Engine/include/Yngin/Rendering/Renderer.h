#pragma once
#include <Yngin/Forward.h>
#include <memory>
#include <glm/vec2.hpp>

namespace Yngin {
	namespace Rendering {
		class Renderer {
		public:
			Context* getContext() const;

			bool isLightingEnabled() const;
			void setLightingEnabled(bool enabled);

			float getRenderDistance() const;
			void setRenderDistance(float renderDistance);

			size_t getSubmeshesRendered() const;

			size_t getMaxSceneLightsCount() const;
			size_t getSceneLightsCount() const;

			uint32_t getGameObjectId(glm::ivec2 pixel);

		private:
			friend class Context;
			friend struct std::default_delete<Renderer>;

			Renderer(Context* ctx);
			~Renderer();

			struct Impl;
			std::unique_ptr<Impl> impl;
		};
	}
}
