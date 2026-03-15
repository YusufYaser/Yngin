#pragma once
#include <stdint.h>

namespace Yngin {
	namespace GameData {
		namespace ScenePak {
			enum OP : uint8_t {
				SCENE,
				GAMEOBJECT,
				COMPONENT,
				CAMERA
			};

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

			struct Operation {
				OP op;
			};

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
				int forcesCount;
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

#pragma pack(pop)
		}
	}
}
