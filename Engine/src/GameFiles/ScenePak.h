#pragma once
#include <stdint.h>
#include <Yngin/UI/Elements/UIElement.h>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>

namespace Yngin {
	namespace GameFiles {
		extern const int VERSION;

		namespace ScenePak {
			enum COMPONENT_TYPE : uint8_t {
				MESH,
				POINT_LIGHT,
				RIGIDBODY,
				BOX_COLLIDER
			};

#pragma pack(push, 1)

			struct Header {
				char magic[11];
				uint8_t version;
			};

			static_assert(sizeof(Header) == 12, "Invalid scene.pak header size");

			struct SceneData {
				int skyboxTexture;
				glm::vec3 ambientLight;
				float gravity;
			};

			struct GameObjectData {
				int id;
				int parent;
				glm::vec3 position;
				glm::vec3 rotation;
				glm::vec3 scale;
			};

			struct ComponentData {
				COMPONENT_TYPE componentType;
			};

			struct MeshData {
				int modelId;
				int textureId;
				glm::vec3 color;
				uint32_t materials[256];
			};

			struct LightData {
				float intensity;
				float distance;
				glm::vec3 color;
			};

			struct RigidBodyData {
				float mass;
				float elasticity;
				bool canBounce;
				glm::vec3 velocity;
				uint8_t forcesCount;
			};

			struct BoxColliderData {
				glm::vec3 offset;
				glm::vec3 size;
			};

			struct CameraData {
				int id;
				glm::vec3 position;
				glm::vec3 orientation;
				float fov;
				float weight;
			};

			struct UIElementData {
				int id;
				int parent;

				UI::UITransform position;
				UI::UITransform size;

				UI::UICrop crop;

				glm::vec4 color;

				glm::vec2 pivot;

				UI_TYPE type;

				// UI Data
			};

			struct UIImageData {
				uint32_t textureId;
			};

			struct UITextData {
				int size;

				uint32_t glyphId;

				glm::ivec2 spacing;
				bool centered[2];

				size_t textLength;

				// char text[textLength]
			};

			struct UIButtonData {
				glm::vec4 hoverColor;
				glm::vec4 clickColor;

				// UIImageData imageData;
				// UITextData textData;
			};

#pragma pack(pop)
		}
	}
}
