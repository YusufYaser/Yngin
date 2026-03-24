#pragma once
#include <stdint.h>
#include <Yngin/UI/Elements/UIElement.h>

namespace Yngin {
	namespace GameFiles {
		extern const int VERSION;

		namespace ScenePak {
			enum COMPONENT_TYPE : uint8_t {
				MESH,
				LIGHT,
				RIGIDBODY,
				BOXCOLLIDER
			};

#pragma pack(push, 1)

			struct Header {
				char magic[11];
				uint8_t version;
			};

			static_assert(sizeof(Header) == 12, "Invalid scene.pak header size");

			struct SceneData {
				int skyboxTexture;
				float ambientLight[3];
				float gravity;
			};

			struct GameObjectData {
				int id;
				int parent;
				float position[3];
				float rotation[3];
				float scale[3];
			};

			struct ComponentData {
				COMPONENT_TYPE componentType;
			};

			struct MeshData {
				int modelId;
				int textureId;
				float color[3];
			};

			struct LightData {
				float intensity;
				float distance;
				float color[3];
			};

			struct RigidBodyData {
				float mass;
				float elasticity;
				bool canBounce;
				float velocity[3];
				uint8_t forcesCount;
			};

			struct BoxColliderData {
				float offset[3];
				float size[3];
			};

			struct CameraData {
				int id;
				float position[3];
				float orientation[3];
				float fov;
				float weight;
			};

			struct UIElementData {
				int id;
				int parent;

				float positionScale[2];
				int positionOffset[2];

				float sizeScale[2];
				int sizeOffset[2];

				float cropStart[2];
				float cropEnd[2];

				float color[4];

				float pivot[2];

				UI_TYPE type;

				// UI Data
			};

			struct UIImageData {
				uint32_t textureId;
			};

			struct UITextData {
				int size;

				uint32_t glyphId;

				int spacing[2];
				bool centered[2];

				size_t textLength;

				// char text[textLength]
			};

			struct UIButtonData {
				float hoverColor[4];
				float clickColor[4];

				// UIImageData imageData;
				// UITextData textData;
			};

#pragma pack(pop)
		}
	}
}
