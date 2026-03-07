#pragma once
#include <memory>
#include <glm/vec2.hpp>

namespace Yngin {
	class Context;

	struct WindowSettings {
		glm::ivec2 size = { 800, 600 };
		bool fullScreen = false;
		glm::ivec2 position = { 100, 100 };
		const char* title = "Yngin Context";
	};

	class Window {
	public:
		void setTitle(const char* title);
		const char* getTitle();

		void setSize(glm::ivec2 size);
		glm::ivec2 getSize();

		void setPosition(glm::ivec2);
		glm::ivec2 getPosition();

		void setCursorLocked(bool locked);
		bool isCursorLocked();

		void setFullscreen(bool fullscreen);
		bool isFullscreen();

	private:
		friend class Context;
		friend struct std::default_delete<Window>;
		friend class InputSystem;

		Window(Context* ctx, const WindowSettings& settings = {});
		~Window();

		struct Impl;
		std::unique_ptr<Impl> impl;
	};
}
