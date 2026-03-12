#pragma once
#include <memory>
#include <glm/vec2.hpp>
#include <glm/vec4.hpp>

struct GLFWwindow;

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
		GLFWwindow* getGLFWwindow() const;

		void setTitle(const char* title);
		const char* getTitle() const;

		void setSize(glm::ivec2 size);
		glm::ivec2 getSize() const;

		void maximize();
		void minimize();

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
