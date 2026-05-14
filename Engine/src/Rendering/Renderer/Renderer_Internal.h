#pragma once
#include <Yngin/Rendering/Renderer.h>
#include <Yngin/Components/Mesh.h>
#include <map>
#include "../../Core/Models/Models_Internal.h"
#include <glm/matrix.hpp>
#include <vector>

namespace Yngin {
	class Scene;
	class GameObject;

	namespace UI {
		class UIElement;
	}

	namespace Rendering {
		constexpr int SSBO_GROW_UNIT = 128;
		constexpr int MAX_LIGHTS = 256;

		struct ShaderLight {
			glm::vec3 position;
			float _pad1;
			glm::vec3 color;
			float _pad2;
			float distance;
			float intensity;
			float _pads[2];
		};

		struct ShaderLightsSSBOData {
			int lightsCount = 0;
			int _pads[3];
			ShaderLight lights[MAX_LIGHTS];
		};

		// View-Frustum Culling

		struct Plane {
			glm::vec3 normal = { 0.0f, 0.0f, 1.0f };
			float distance = 0.0f;
		};

		// Instancing

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
			uint32_t objectId;
			int isLight;
			int _pads[2];
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

			Plane frustumPlanes[6];

			size_t sceneSubmeshesRendered = 0;
			size_t sceneLights = 0;

			void render(Scene* scene);
			// renderChildrenDepth = -1 for infinity
			void render(GameObject* gameObject, int renderChildrenDepth = 0);
			// renderChildrenDepth = -1 for infinity
			void render(UI::UIElement* element, int renderChildrenDepth = 0);

			void render(Components::Mesh* mesh);

			void renderSubmeshInstanced(InternalSubmesh* submesh, uint32_t texId, const InstancePrepData& data);

			bool preparingInstances = false;
			std::map<std::pair<InternalSubmesh*, uint32_t>, InstancePrepData> instancesPrep;

			size_t ssboSize = 0;
			// max instances supported by the GPU
			size_t maxInstances = 1;

			GLuint vertexSSBO;
			GLuint fragSSBO;
			GLuint lightsSSBO;

			glm::ivec2 renderedViewportSize;
			GLuint FBO;
			GLuint colorsTex;
			GLuint IDsTex;
			GLuint RBO;
		};
	}
}
