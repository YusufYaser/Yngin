#pragma once
#include <memory>

namespace Yngin {
	class Context;

	class Window {
	private:
		friend class Context;
		friend struct std::default_delete<Window>;

		Window(Context* ctx);
		~Window();

		struct Impl;
		std::unique_ptr<Impl> impl;
	};
}
