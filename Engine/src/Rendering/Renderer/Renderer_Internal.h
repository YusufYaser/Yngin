#pragma once
#include <Yngin/Rendering/Renderer.h>
#include <Yngin/Components/Mesh.h>
#include <map>
#include "../../Core/Models/Models_Internal.h"
#include <glm/matrix.hpp>
#include <vector>

constexpr int SSBO_GROW_UNIT = 128;

namespace Yngin {
	class Scene;
	class GameObject;

	namespace UI {
		class UIElement;
	}

	namespace Rendering {
		struct InstanceOffsetMaterial {
			glm::vec3 ambientColor = glm::vec3(1.0f);
			float _pad1;
			glm::vec3 diffuseColor = glm::vec3(1.0f);
			float _pad2;
			glm::vec3 specularColor = glm::vec3(1.0f);
			float _pad3;
			float specularComponent = 64.0f;
			float _pads[3];
		};

		struct InstanceVertexOffset {
			glm::mat4 model;
			glm::mat4 normalMatrix;
		};

		struct InstanceFragmentOffset {
			glm::vec4 color;
			InstanceOffsetMaterial material;
			int isLight;
			int _pads[3];
		};

		struct InstancePrepData {
			int instances = 0;
			std::vector<InstanceVertexOffset> vOffsets;
			std::vector<InstanceFragmentOffset> fOffsets;
		};

		struct Renderer::Impl {
			Context* ctx;

			bool lightingEnabled = true;

			float renderDistance = 512.0f;

			void render(Scene* scene);
			// renderChildrenDepth = -1 for infinity
			void render(GameObject* gameObject, int renderChildrenDepth = 0);
			// renderChildrenDepth = -1 for infinity
			void render(UI::UIElement* element, int renderChildrenDepth = 0);

			void render(Components::Mesh* mesh);

			void renderSubmeshInstanced(InternalSubmesh* submesh, Texture* tex, const InstancePrepData& data);

			bool preparingInstances = false;
			std::map<std::pair<InternalSubmesh*, Texture*>, InstancePrepData> instancesPrep;

			size_t ssboSize = 0;
			// max instances supported by the GPU
			size_t maxInstances = 1;

			GLuint vertexSSBO;
			GLuint fragSSBO;
		};
	}
}
