#pragma once
#include <stdint.h>

struct GLFWwindow;

namespace Yngin {
	enum class CONTEXT_STATUS : uint8_t;

	enum class MOUSE_BUTTON : uint8_t;
	enum class KEY : uint8_t;

	enum class MODEL_FRONT_FACE : uint8_t;
	enum class MODEL_FILE_TYPE : uint8_t;

	enum class SHADER_TYPE : uint8_t;

	enum class TEXTURE_WRAP : uint8_t;
	enum class TEXTURE_FILTER : uint8_t;

	enum class UI_TYPE : uint8_t;

	enum class COLLIDER_TYPE : uint8_t;

	struct ContextSettings;
	struct Vertex;
	struct ModelData;
	struct LightSettings;
	struct WindowSettings;
	struct ShaderSource;
	struct TextureSettings;
	struct TextureData;

	class Context;
	class GameObject;
	class GameObjectsManager;
	class InputSystem;
	class Material;
	class MaterialsManager;
	class Model;
	class ModelsManager;
	class Scene;
	class ScenesManager;
	class Script;
	class ScriptsManager;
	class Window;

	class Camera;
	class CamerasManager;
	class Shader;
	class ShadersManager;
	class Texture;
	class TexturesManager;

	class Meta;

	namespace Rendering {
		class Renderer;
	}

	namespace Services {
		enum class TWEEN_FUNCTION : uint8_t;

		struct TweenSettings;

		class Service;
		class Tween;
	}

	namespace UI {
		struct UITransform;
		struct UICrop;

		class UIManager;
		class UIElement;

		class Text;
		class Image;
		class Button;
	}

	namespace Physics {
		extern const float SMALLEST_UNIT;

		struct Ray;

		class PhysicsEngine;
	}

	namespace Components {
		class Component;

		class Collider;
		class BoxCollider;
		class Light;
		class Mesh;
		class RigidBody;
	}
}
