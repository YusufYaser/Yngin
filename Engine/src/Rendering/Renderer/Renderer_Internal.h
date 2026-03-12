#pragma once
#include <Yngin/Rendering/Renderer.h>
#include <Yngin/Components/Mesh.h>

namespace Yngin {
	class Scene;
	class GameObject;

	namespace UI {
		class UIElement;
	}

	namespace Rendering {
		struct Renderer::Impl {
			Context* ctx;

			float renderDistance = 512.0f;

			void render(Scene* scene);
			// renderChildrenDepth = -1 for infinity
			void render(GameObject* gameObject, int renderChildrenDepth = 0);
			// renderChildrenDepth = -1 for infinity
			void render(UI::UIElement* element, int renderChildrenDepth = 0);

			void render(Components::Mesh* mesh);
		};
	}
}
