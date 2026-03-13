#pragma once
#include <memory>

namespace Yngin {
	class Context;

	namespace Rendering {
		class Renderer {
		public:
			Context* getContext() const;

			bool isLightingEnabled() const;
			void setLightingEnabled(bool enabled);

			float getRenderDistance() const;
			void setRenderDistance(float renderDistance);

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
