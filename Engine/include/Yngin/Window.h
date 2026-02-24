#pragma once
#include <memory>

namespace Yngin {
	class Context;

	class Window {
	public:
		void update();
		void swapBuffers();
		bool shouldClose();

	private:
		friend class Context;
		friend struct std::default_delete<Window>;

		Window(Context* ctx);
		~Window();

		struct Impl;
		std::unique_ptr<Impl> impl;
	};
}
