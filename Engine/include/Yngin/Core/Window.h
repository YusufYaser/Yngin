#pragma once
#include <memory>

namespace Yngin {
	class Context;

	struct WindowSettings {
		int width = 800;
		int height = 600;
		const char* title = "Yngin Context";
	};

	class Window {
	public:
		void setTitle(const char* title);
		void setSize(int width, int height);
		void setPosition(int x, int y);
		void setCursorLocked(bool locked);

	private:
		friend class Context;
		friend struct std::default_delete<Window>;

		Window(Context* ctx, const WindowSettings& settings = {});
		~Window();

		struct Impl;
		std::unique_ptr<Impl> impl;
	};
}
