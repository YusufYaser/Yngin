#pragma once
#include <memory>

namespace Yngin {
	class Context;

	class Window {
	public:
		void setTitle(const char* title);
		void setSize(int width, int height);
		void setPosition(int x, int y);
		void setCursorLocked(bool locked);

	private:
		friend class Context;
		friend struct std::default_delete<Window>;

		Window(Context* ctx);
		~Window();

		struct Impl;
		std::unique_ptr<Impl> impl;
	};
}
