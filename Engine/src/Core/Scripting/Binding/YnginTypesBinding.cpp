#include "../Scripting_Internal.h"
#include <Yngin/Yngin.h>

namespace Yngin {
	void ScriptsManager::Impl::bindYnginTypes() {
		lua.new_enum<MOUSE_BUTTON>("MOUSE_BUTTON", {
			{ "LEFT", MOUSE_BUTTON::LEFT },
			{ "RIGHT", MOUSE_BUTTON::RIGHT },
			{ "MIDDLE", MOUSE_BUTTON::MIDDLE }
			});

		lua.new_enum<KEY>("KEY", {
			{"UNKNOWN",   KEY::UNKNOWN},
			{"A",         KEY::A}, {"B",         KEY::B},
			{"C",         KEY::C}, {"D",         KEY::D},
			{"E",         KEY::E}, {"F",         KEY::F},
			{"G",         KEY::G}, {"H",         KEY::H},
			{"I",         KEY::I}, {"J",         KEY::J},
			{"K",         KEY::K}, {"L",         KEY::L},
			{"M",         KEY::M}, {"N",         KEY::N},
			{"O",         KEY::O}, {"P",         KEY::P},
			{"Q",         KEY::Q}, {"R",         KEY::R},
			{"S",         KEY::S}, {"T",         KEY::T},
			{"U",         KEY::U}, {"V",         KEY::V},
			{"W",         KEY::W}, {"X",         KEY::X},
			{"Y",         KEY::Y}, {"Z",         KEY::Z},

			{"NUM_0",     KEY::NUM_0}, {"NUM_1",     KEY::NUM_1},
			{"NUM_2",     KEY::NUM_2}, {"NUM_3",     KEY::NUM_3},
			{"NUM_4",     KEY::NUM_4}, {"NUM_5",     KEY::NUM_5},
			{"NUM_6",     KEY::NUM_6}, {"NUM_7",     KEY::NUM_7},
			{"NUM_8",     KEY::NUM_8}, {"NUM_9",     KEY::NUM_9},

			{"F1",        KEY::F1}, {"F2",        KEY::F2},
			{"F3",        KEY::F3}, {"F4",        KEY::F4},
			{"F5",        KEY::F5}, {"F6",        KEY::F6},
			{"F7",        KEY::F7}, {"F8",        KEY::F8},
			{"F9",        KEY::F9}, {"F10",       KEY::F10},
			{"F11",       KEY::F11}, {"F12",       KEY::F12},
			{"F13",       KEY::F13}, {"F14",       KEY::F14},
			{"F15",       KEY::F15}, {"F16",       KEY::F16},
			{"F17",       KEY::F17}, {"F18",       KEY::F18},
			{"F19",       KEY::F19}, {"F20",       KEY::F20},
			{"F21",       KEY::F21}, {"F22",       KEY::F22},
			{"F23",       KEY::F23}, {"F24",       KEY::F24},
			{"F25",       KEY::F25},

			{"SPACE",     KEY::SPACE}, {"ESCAPE",    KEY::ESCAPE},
			{"ENTER",     KEY::ENTER}, {"TAB",       KEY::TAB},
			{"BACKSPACE", KEY::BACKSPACE},

			{"RIGHT",     KEY::RIGHT}, {"LEFT",      KEY::LEFT},
			{"DOWN",      KEY::DOWN}, {"UP",        KEY::UP},

			{"LSHIFT",    KEY::LSHIFT}, {"LCTRL",     KEY::LCTRL},
			{"LALT",      KEY::LALT}, {"LSUPER",    KEY::LSUPER},
			{"RSHIFT",    KEY::RSHIFT}, {"RCTRL",     KEY::RCTRL},
			{"RALT",      KEY::RALT}, {"RSUPER",    KEY::RSUPER}
			});


		// Context
		lua.new_usertype<Context>("Context",
			sol::no_constructor,

			BIND(Context, getFrame),
			BIND(Context, getMaxFPS),
			BIND(Context, setMaxFPS),

			BIND(Context, getFrameStartTime)
		);


		// Renderer
		lua.new_usertype<Rendering::Renderer>("Renderer",
			sol::no_constructor,

			BIND(Rendering::Renderer, isLightingEnabled),
			BIND(Rendering::Renderer, setLightingEnabled),

			BIND(Rendering::Renderer, getRenderDistance),
			BIND(Rendering::Renderer, setRenderDistance)
		);


		// Textures
		lua.new_usertype<Texture>("Texture",
			sol::no_constructor,

			// TODO: needs to be completed
			BIND(Texture, getId),

			BIND(Texture, getSize)
		);

		lua.new_usertype<TexturesManager>("TexturesManager",
			sol::no_constructor,

			// TODO: needs to be completed
			BIND(TexturesManager, deleteTexture),

			BIND(TexturesManager, getTexturesCount),
			BIND(TexturesManager, getTextures),
			BIND(TexturesManager, getTexture)
		);

		// GameObjects
		lua.new_usertype<GameObject>("GameObject",
			sol::no_constructor,

			// TODO: needs to be completed
			BIND(GameObject, getId),

			BIND(GameObject, getParent),
			"setParent", sol::overload(
				[](GameObject& self, uint32_t parentId) { self.setParent(parentId); },

				[](GameObject& self, GameObject* newParent) { self.setParent(newParent); }
			),

			BIND(GameObject, getChildren),

			"createChild", sol::overload(
				static_cast<GameObject * (GameObject::*)()>(&GameObject::createChild),

				[](GameObject& self, uint32_t id) {
					return self.createChild(id);
				},

				static_cast<GameObject * (GameObject::*)(uint32_t, bool)>(&GameObject::createChild)
			),

			"deleteChild", sol::overload(
				static_cast<void(GameObject::*)(uint32_t)>(&GameObject::deleteChild),

				static_cast<void(GameObject::*)(GameObject*)>(&GameObject::deleteChild)
			),

			"moveChild", sol::overload(
				static_cast<void(GameObject::*)(uint32_t, GameObject*)>(&GameObject::moveChild),

				static_cast<void(GameObject::*)(GameObject*, GameObject*)>(&GameObject::moveChild),

				static_cast<void(GameObject::*)(GameObject*, uint32_t)>(&GameObject::moveChild),

				static_cast<void(GameObject::*)(uint32_t, uint32_t)>(&GameObject::moveChild)
			),

			BIND(GameObject, getPosition),
			BIND(GameObject, setPosition),
			BIND(GameObject, getRotation),
			BIND(GameObject, setRotation),
			BIND(GameObject, getScale),
			BIND(GameObject, setScale)
		);

		lua.new_usertype<GameObjectsManager>("GameObjectsManager",
			sol::no_constructor,

			BIND(GameObjectsManager, getRootGameObject),
			BIND(GameObjectsManager, getGameObject),

			BIND(GameObjectsManager, getGameObjectsCount),
			BIND(GameObjectsManager, getGameObjects),

			"deleteGameObject", sol::overload(
				static_cast<void(GameObjectsManager::*)(uint32_t)>(&GameObjectsManager::deleteGameObject),
				static_cast<void(GameObjectsManager::*)(GameObject*)>(&GameObjectsManager::deleteGameObject)
			)
		);


		// Scenes
		lua.new_usertype<Scene>("Scene",
			sol::no_constructor,

			// TODO: needs to be completed
			BIND(Scene, getId),

			BIND(Scene, activate),

			BIND(Scene, getGameObjectsManager),

			BIND(Scene, getGravity),
			BIND(Scene, setGravity)
		);

		lua.new_usertype<ScenesManager>("ScenesManager",
			sol::no_constructor,

			BIND(ScenesManager, getScene),
			BIND(ScenesManager, getScenes),

			"createScene", sol::overload(
				static_cast<Scene * (ScenesManager::*)()>(&ScenesManager::createScene),

				[](ScenesManager& self, uint32_t id) {
					return self.createScene(id);
				},

				static_cast<Scene * (ScenesManager::*)(uint32_t, bool)>(&ScenesManager::createScene)
			),

			"deleteScene", sol::overload(
				static_cast<void(ScenesManager::*)(uint32_t)>(&ScenesManager::deleteScene),
				static_cast<void(ScenesManager::*)(Scene*)>(&ScenesManager::deleteScene)
			),

			BIND(ScenesManager, getActive),

			"setActive", sol::overload(
				static_cast<void(ScenesManager::*)(uint32_t)>(&ScenesManager::setActive),
				static_cast<void(ScenesManager::*)(Scene*)>(&ScenesManager::setActive)
			)
		);


		// Cameras
		lua.new_usertype<Camera>("Camera",
			sol::no_constructor,

			BIND(Camera, getId),

			BIND(Camera, getPosition),
			BIND(Camera, getOrientation),
			BIND(Camera, getFov),

			BIND(Camera, setPosition),
			BIND(Camera, setOrientation),
			BIND(Camera, setFov),

			BIND(Camera, lookAt),

			BIND(Camera, getWeight),
			BIND(Camera, setWeight)
		);

		lua.new_usertype<CamerasManager>("CamerasManager",
			sol::no_constructor,

			"createCamera", sol::overload(
				static_cast<Camera * (CamerasManager::*)()>(&CamerasManager::createCamera),

				[](CamerasManager& self, uint32_t id) {
					return self.createCamera(id);
				},

				static_cast<Camera * (CamerasManager::*)(uint32_t, bool)>(&CamerasManager::createCamera)
			),

			"deleteCamera", sol::overload(
				static_cast<void(CamerasManager::*)(uint32_t)>(&CamerasManager::deleteCamera),
				static_cast<void(CamerasManager::*)(Camera*)>(&CamerasManager::deleteCamera)
			),

			BIND(CamerasManager, getCameras),
			BIND(CamerasManager, getCamera),

			BIND(CamerasManager, getTotalWeight),

			"setActive", sol::overload(
				static_cast<void(CamerasManager::*)(uint32_t)>(&CamerasManager::setActive),
				static_cast<void(CamerasManager::*)(Camera*)>(&CamerasManager::setActive)
			),

			BIND(CamerasManager, getFinalPos),
			BIND(CamerasManager, getFinalOrientation),
			BIND(CamerasManager, getFinalFov)
		);
		lua["Scene"]["getCamerasManager"] = &Scene::getCamerasManager;

		lua.new_usertype<Window>("Window",
			sol::no_constructor,

			BIND(Window, setFocused),
			BIND(Window, isFocused),

			BIND(Window, hasTitleBar),

			BIND(Window, setTitle),
			BIND(Window, getTitle),

			BIND(Window, setSize),
			BIND(Window, getSize),

			BIND(Window, maximize),
			BIND(Window, minimize),

			BIND(Window, setPosition),
			BIND(Window, getPosition),

			BIND(Window, setCursorLocked),
			BIND(Window, isCursorLocked),

			BIND(Window, setFullscreen),
			BIND(Window, isFullscreen)
		);

		lua.new_usertype<InputSystem>("InputSystem",
			sol::no_constructor,

			"getMousePosition", sol::overload(
				[](const InputSystem& self) { return self.getMousePosition(); },

				&InputSystem::getMousePosition
			),

			BIND(InputSystem, setMousePosition),

			BIND(InputSystem, isMousePressed),
			BIND(InputSystem, isMouseJustPressed),
			BIND(InputSystem, isMouseJustReleased),

			BIND(InputSystem, isKeyPressed),
			BIND(InputSystem, isKeyJustPressed),
			BIND(InputSystem, isKeyJustReleased)
		);


		// Physics
		lua.new_usertype<Physics::Ray>("Ray",
			sol::constructors<
			Physics::Ray()
			>(),

			BIND(Physics::Ray, origin),
			BIND(Physics::Ray, direction)
		);

		lua.new_usertype<Physics::PhysicsEngine>("PhysicsEngine",
			sol::no_constructor,

			// TODO: needs to be completed after adding components
			BIND(Physics::PhysicsEngine, isSimulationEnabled),
			BIND(Physics::PhysicsEngine, setSimulationEnabled),

			BIND(Physics::PhysicsEngine, getSimulationDistance),
			BIND(Physics::PhysicsEngine, setSimulationDistance)
		);


		// Services
		lua.new_usertype<Services::Service>("Service",
			sol::no_constructor
		);

		lua.new_enum<Services::TWEEN_FUNCTION>("TWEEN_FUNCTION", {
			{ "LINEAR", Services::TWEEN_FUNCTION::LINEAR },
			{ "EASE_IN", Services::TWEEN_FUNCTION::EASE_IN },
			{ "EASE_OUT", Services::TWEEN_FUNCTION::EASE_OUT },
			{ "EASE_INOUT", Services::TWEEN_FUNCTION::EASE_INOUT }
			});

		lua.new_usertype<Services::TweenSettings>("TweenSettings",
			sol::constructors<
			Services::TweenSettings()
			>(),

			BIND(Services::TweenSettings, duration),
			"tweenFunction", &Services::TweenSettings::function
		);

		lua.new_usertype<Services::Tween>("Tween",
			sol::no_constructor,

			BIND(Services::Tween, cancel),
			BIND(Services::Tween, isActive),

			// TODO: add tweenFloat
			"tweenPos", sol::overload(
				static_cast<int(Services::Tween::*)(GameObject*, glm::vec3, const Services::TweenSettings&)>(&Services::Tween::tweenPos),

				static_cast<int(Services::Tween::*)(Camera*, glm::vec3, const Services::TweenSettings&)>(&Services::Tween::tweenPos)
			),

			BIND(Services::Tween, isPaused),
			BIND(Services::Tween, setPaused)
		);

		// TODO: add models
		// TODO: add shaders
		// TODO: add scripts
		// TODO: add components
		// TODO: add UI
	}
}
