#pragma once
#include <Yngin/Forward.h>
#include <memory>
#include <glm/vec2.hpp>
#include <glm/vec4.hpp>

namespace Yngin {
	struct WindowSettings {
		glm::ivec2 size = { 800, 600 };
		bool fullScreen = false;
		glm::ivec2 position = { 100, 100 };
		char title[32] = "Yngin Context";
		bool hasTitleBar = true;
	};

	class Window {
	public:
		Context* getContext() const;

		GLFWwindow* getGLFWwindow() const;

		bool getWindowVisible() const;
		void showWindow();
		void hideWindow();

		void setFocused();
		bool isFocused() const;

		bool hasTitleBar() const;

		void setTitle(const char* title);
		const char* getTitle() const;

		void setSize(glm::ivec2 size);
		glm::ivec2 getSize() const;

		bool isMaximized() const;
		void maximize();
		void restore();
		void minimize();

		void setPositionCentered();
		void setPosition(glm::ivec2);
		glm::ivec2 getPosition() const;

		void setCursorLocked(bool locked);
		bool isCursorLocked() const;

		void setMousePosition(glm::ivec2 pos);

		void setFullscreen(bool fullscreen);
		bool isFullscreen() const;

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
