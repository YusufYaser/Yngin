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

		lua.new_usertype<Meta>("Meta",
			sol::constructors<
			Meta()
			>(),

			BIND(Meta, reset),
			BIND(Meta, clearMeta),

			"setMeta", sol::overload(
				static_cast<void(Meta::*)(const std::string&, const std::string&)>(&Meta::setMeta),
				static_cast<void(Meta::*)(const std::string&, const int&)>(&Meta::setMeta),
				static_cast<void(Meta::*)(const std::string&, const float&)>(&Meta::setMeta)
			),

			BIND(Meta, getMetasCount),

			BIND(Meta, getMetas),
			BIND(Meta, getMeta),

			BIND(Meta, getMetaString),
			BIND(Meta, getMetaInt),
			BIND(Meta, getMetaFloat)
		);


		// Context
		lua.new_usertype<Context>("Context",
			sol::no_constructor,

			BIND(Context, meta),

			BIND(Context, getFrame),
			BIND(Context, getMaxFPS),
			BIND(Context, setMaxFPS),

			BIND(Context, getFrameStartTime),

			BIND(Context, getGlobalUIManager)
		);


		// Renderer
		lua.new_usertype<Rendering::Renderer>("Renderer",
			sol::no_constructor,

			BIND(Rendering::Renderer, isLightingEnabled),
			BIND(Rendering::Renderer, setLightingEnabled),

			BIND(Rendering::Renderer, getRenderDistance),
			BIND(Rendering::Renderer, setRenderDistance),

			BIND(Rendering::Renderer, getSubmeshesRendered),

			BIND(Rendering::Renderer, getSceneLightsCount),

			BIND(Rendering::Renderer, getGameObjectId)
		);


		// Textures
		lua.new_usertype<Texture>("Texture",
			sol::no_constructor,

			BIND(Texture, meta),

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


		// Models
		lua.new_enum<MODEL_FRONT_FACE>("MODEL_FRONT_FACE", {
			{ "NONE", MODEL_FRONT_FACE::NONE },
			{ "CCW", MODEL_FRONT_FACE::CCW },
			{ "CW", MODEL_FRONT_FACE::CW },
			});

		lua.new_usertype<ModelData>("ModelData",
			sol::constructors<
			ModelData()
			>(),

			BIND(ModelData, vertices),
			BIND(ModelData, indices),
			BIND(ModelData, frontFace)
		);

		lua.new_usertype<Model>("Model",
			sol::no_constructor,

			BIND(Model, meta),

			BIND(Model, getId),

			BIND(Model, getSubmeshesCount),
			BIND(Model, getModelData)
		);

		lua.new_usertype<ModelsManager>("ModelsManager",
			sol::no_constructor,

			"createModel", sol::overload(
				static_cast<Model * (ModelsManager::*)(const ModelData&)>(&ModelsManager::createModel),

				static_cast<Model * (ModelsManager::*)(const MODEL_FILE_TYPE&, const char*, size_t)>(&ModelsManager::createModel),


				[](ModelsManager& self, const ModelData& data, uint16_t id) {
					return self.createModel(data, id);
				},

				[](ModelsManager& self, const ModelData& data, uint16_t id, bool override) {
					return self.createModel(data, id, override);
				},

				[](ModelsManager& self, const MODEL_FILE_TYPE& type, const char* data, size_t length, uint16_t id) {
					return self.createModel(type, data, length, id);
				},

				[](ModelsManager& self, const MODEL_FILE_TYPE& type, const char* data, size_t length, uint16_t id, bool override) {
					return self.createModel(type, data, length, id, override);
				}
			),

			"deleteModel", sol::overload(
				static_cast<void(ModelsManager::*)(uint16_t)>(&ModelsManager::deleteModel),
				static_cast<void(ModelsManager::*)(Model*)>(&ModelsManager::deleteModel)
			),

			BIND(ModelsManager, getModelsCount),
			BIND(ModelsManager, getModels),

			BIND(ModelsManager, getModel)
		);


		// GameObjects
		lua.new_usertype<GameObject>("GameObject",
			sol::no_constructor,

			BIND(GameObject, meta),

			BIND(GameObject, getId),
			BIND(GameObject, getScene),

			BIND(GameObject, getParent),
			"setParent", sol::overload(
				[](GameObject& self, uint32_t parentId) { self.setParent(parentId); },

				[](GameObject& self, GameObject* newParent) { self.setParent(newParent); }
			),

			BIND(GameObject, getChildren),

			BIND(GameObject, getChild),

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
			BIND(GameObject, setScale),

			"hasComponent", &GameObject::hasComponent<Components::Component>,
			"hasComponentMesh", &GameObject::hasComponent<Components::Mesh>,
			"hasComponentPointLight", &GameObject::hasComponent<Components::PointLight>,
			"hasComponentDirectionalLight", &GameObject::hasComponent<Components::DirectionalLight>,
			"hasComponentBoxCollider", &GameObject::hasComponent<Components::BoxCollider>,
			"hasComponentRigidBody", &GameObject::hasComponent<Components::RigidBody>,

			"getComponent", &GameObject::getComponent<Components::Component>,
			"getComponentMesh", &GameObject::getComponent<Components::Mesh>,
			"getComponentPointLight", &GameObject::getComponent<Components::PointLight>,
			"getComponentDirectionalLight", &GameObject::getComponent<Components::DirectionalLight>,
			"getComponentBoxCollider", &GameObject::getComponent<Components::BoxCollider>,
			"getComponentRigidBody", &GameObject::getComponent<Components::RigidBody>,

			"createComponent", &GameObject::createComponent<Components::Component>,
			"createComponentMesh", &GameObject::createComponent<Components::Mesh>,
			"createComponentPointLight", &GameObject::createComponent<Components::PointLight>,
			"createComponentDirectionalLight", &GameObject::createComponent<Components::DirectionalLight>,
			"createComponentBoxCollider", &GameObject::createComponent<Components::BoxCollider>,
			"createComponentRigidBody", &GameObject::createComponent<Components::RigidBody>,

			"deleteComponent", &GameObject::deleteComponent<Components::Component>,
			"deleteComponentMesh", &GameObject::deleteComponent<Components::Mesh>,
			"deleteComponentPointLight", &GameObject::deleteComponent<Components::PointLight>,
			"deleteComponentDirectionalLight", &GameObject::deleteComponent<Components::DirectionalLight>,
			"deleteComponentBoxCollider", &GameObject::deleteComponent<Components::BoxCollider>,
			"deleteComponentRigidBody", &GameObject::deleteComponent<Components::RigidBody>
		);

		lua.new_usertype<GameObjectsManager>("GameObjectsManager",
			sol::no_constructor,

			BIND(GameObjectsManager, getRootGameObject),
			BIND(GameObjectsManager, getGameObject),

			BIND(GameObjectsManager, getGameObjectsCount),
			BIND(GameObjectsManager, getGameObjects),

			"createGameObject", sol::overload(
				static_cast<GameObject * (GameObjectsManager::*)()>(&GameObjectsManager::createGameObject),

				[](GameObjectsManager& self, uint32_t id) {
					return self.createGameObject(id);
				},

				static_cast<GameObject * (GameObjectsManager::*)(uint32_t, bool)>(&GameObjectsManager::createGameObject)
			),

			"deleteGameObject", sol::overload(
				static_cast<void(GameObjectsManager::*)(uint32_t)>(&GameObjectsManager::deleteGameObject),
				static_cast<void(GameObjectsManager::*)(GameObject*)>(&GameObjectsManager::deleteGameObject)
			)
		);


		// Scenes
		lua.new_usertype<Scene>("Scene",
			sol::no_constructor,

			BIND(Scene, meta),

			// TODO: needs to be completed
			BIND(Scene, getId),

			BIND(Scene, activate),

			BIND(Scene, getGameObjectsManager),

			BIND(Scene, getGravity),
			BIND(Scene, setGravity),

			BIND(Scene, getUIManager),
			BIND(Scene, getCamerasManager),

			BIND(Scene, getSkyboxTextureId),

			"setSkyboxTexture", sol::overload(
				static_cast<void(Scene::*)(uint32_t)>(&Scene::setSkyboxTexture),
				static_cast<void(Scene::*)(Texture*)>(&Scene::setSkyboxTexture)
			)
		);

		lua.new_usertype<ScenesManager>("ScenesManager",
			sol::no_constructor,

			BIND(ScenesManager, getScenesCount),

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

			BIND(Camera, meta),

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

			BIND(CamerasManager, getBlendedCamera),

			BIND(CamerasManager, getCameras),
			BIND(CamerasManager, getCamera),

			BIND(CamerasManager, getTotalWeight),

			"setActive", sol::overload(
				static_cast<void(CamerasManager::*)(uint32_t)>(&CamerasManager::setActive),
				static_cast<void(CamerasManager::*)(Camera*)>(&CamerasManager::setActive)
			)
		);

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

			BIND(Physics::PhysicsEngine, isSimulationEnabled),
			BIND(Physics::PhysicsEngine, setSimulationEnabled),

			BIND(Physics::PhysicsEngine, getSimulationDistance),
			BIND(Physics::PhysicsEngine, setSimulationDistance),

			"raycast", sol::overload(
				[](const Physics::PhysicsEngine& self, Scene* scene, const Physics::Ray& ray) {
					return self.raycast(scene, ray);
				},

				static_cast<Components::Collider * (Physics::PhysicsEngine::*)(Scene*, const Physics::Ray&, float) const>(&Physics::PhysicsEngine::raycast)
			)
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
			sol::base_classes, sol::bases<Services::Service>(),

			BIND(Services::Tween, cancel),
			BIND(Services::Tween, isActive),

			BIND(Services::Tween, tweenFloat),

			// TODO: add tweenFloat
			"tweenPos", sol::overload(
				static_cast<int(Services::Tween::*)(GameObject*, glm::vec3, const Services::TweenSettings&)>(&Services::Tween::tweenPos),

				static_cast<int(Services::Tween::*)(Camera*, glm::vec3, const Services::TweenSettings&)>(&Services::Tween::tweenPos)
			),

			BIND(Services::Tween, isPaused),
			BIND(Services::Tween, setPaused)
		);


		// UI
		lua.new_enum<UI_TYPE>("UI_TYPE", {
			{ "NONE", UI_TYPE::NONE },
			{ "IMAGE", UI_TYPE::IMAGE },
			{ "TEXT", UI_TYPE::TEXT },
			{ "BUTTON", UI_TYPE::BUTTON },
			});

		lua.new_usertype<UI::UITransform>("UITransform",
			sol::no_constructor,

			BIND(UI::UITransform, xScale),
			BIND(UI::UITransform, xOffset),
			BIND(UI::UITransform, yScale),
			BIND(UI::UITransform, yOffset)
		);

		lua.new_usertype<UI::UICrop>("UICrop",
			sol::no_constructor,

			BIND(UI::UICrop, start),
			BIND(UI::UICrop, end)
		);

		lua.new_usertype<UI::UIElement>("UIElement",
			sol::no_constructor,

			BIND(UI::UIElement, meta),

			BIND(UI::UIElement, getId),
			BIND(UI::UIElement, getScene),
			BIND(UI::UIElement, getType),

			"getParent", [](const UI::UIElement& self) { return self.getParent(); },
			"getParentImage", &UI::UIElement::getParent<UI::Image>,
			"getParentText", &UI::UIElement::getParent<UI::Text>,
			"getParentButton", &UI::UIElement::getParent<UI::Button>,

			"getChild", [](const UI::UIElement& self, uint32_t id) { return self.getChild(id); },
			"getChildImage", &UI::UIElement::getChild<UI::Image>,
			"getChildText", &UI::UIElement::getChild<UI::Text>,
			"getChildButton", &UI::UIElement::getChild<UI::Button>,

			// TODO: add custom IDs
			"createChild", [](UI::UIElement& self) { return self.createChild(); },
			"createChildImage", &UI::UIElement::createChild<UI::Image>,
			"createChildText", &UI::UIElement::createChild<UI::Text>,
			"createChildButton", &UI::UIElement::createChild<UI::Button>,

			BIND(UI::UIElement, getChildren),

			"deleteChild", sol::overload(
				static_cast<void(UI::UIElement::*)(uint32_t)>(&UI::UIElement::deleteChild),
				static_cast<void(UI::UIElement::*)(UI::UIElement*)>(&UI::UIElement::deleteChild)
			),

			"moveChild", sol::overload(
				static_cast<void(UI::UIElement::*)(uint32_t, UI::UIElement*)>(&UI::UIElement::moveChild),

				static_cast<void(UI::UIElement::*)(UI::UIElement*, UI::UIElement*)>(&UI::UIElement::moveChild),

				static_cast<void(UI::UIElement::*)(UI::UIElement*, uint32_t)>(&UI::UIElement::moveChild),

				static_cast<void(UI::UIElement::*)(uint32_t, uint32_t)>(&UI::UIElement::moveChild)
			),

			BIND(UI::UIElement, setPosition),
			BIND(UI::UIElement, getPosition),

			BIND(UI::UIElement, setSize),
			BIND(UI::UIElement, getSize),

			BIND(UI::UIElement, isHovered),

			"isClicked", sol::overload(
				[](const UI::UIElement& self) {
					return self.isClicked();
				},

				static_cast<bool(UI::UIElement::*)(const MOUSE_BUTTON&) const>(&UI::UIElement::isClicked)
			),
			"isHeld", sol::overload(
				[](const UI::UIElement& self) {
					return self.isHeld();
				},

				static_cast<bool(UI::UIElement::*)(const MOUSE_BUTTON&) const>(&UI::UIElement::isHeld)
			),

			BIND(UI::UIElement, setCrop),
			BIND(UI::UIElement, getCrop),

			BIND(UI::UIElement, setColor),
			BIND(UI::UIElement, getColor),

			BIND(UI::UIElement, setPivot),
			BIND(UI::UIElement, getPivot)
		);

		lua.new_usertype<UI::Image>("Image",
			sol::no_constructor,
			sol::base_classes, sol::bases<UI::UIElement>(),

			"setTexture", sol::overload(
				static_cast<void(UI::Image::*)(uint32_t)>(&UI::Image::setTexture),
				static_cast<void(UI::Image::*)(Texture*)>(&UI::Image::setTexture)
			),
			BIND(UI::Image, getTexture)
		);

		lua.new_usertype<UI::Text>("Text",
			sol::no_constructor,
			sol::base_classes, sol::bases<UI::UIElement>(),

			BIND(UI::Text, setText),
			BIND(UI::Text, getText),

			BIND(UI::Text, setTextSize),
			BIND(UI::Text, getTextSize),

			"setGlyph", sol::overload(
				static_cast<void(UI::Text::*)(uint32_t)>(&UI::Text::setGlyph),
				static_cast<void(UI::Text::*)(Texture*)>(&UI::Text::setGlyph)
			),
			BIND(UI::Text, getGlyph),

			BIND(UI::Text, setSpacing),
			BIND(UI::Text, getSpacing),

			BIND(UI::Text, getTextDimensions),
			BIND(UI::Text, getTextDimensionsPixels),

			BIND(UI::Text, setTextCentered),
			BIND(UI::Text, isTextCentered)
		);

		lua.new_usertype<UI::Button>("Button",
			sol::no_constructor,
			sol::base_classes, sol::bases<UI::UIElement>(),

			BIND(UI::Button, getTextElement),
			BIND(UI::Button, getImage),

			BIND(UI::Button, getHoverColor),
			BIND(UI::Button, setHoverColor),

			BIND(UI::Button, getClickColor),
			BIND(UI::Button, setClickColor)
		);

		lua.new_usertype<UI::UIManager>("UIManager",
			sol::no_constructor,

			BIND(UI::UIManager, getRootElement),
			BIND(UI::UIManager, getElement),

			BIND(UI::UIManager, getElementsCount),
			BIND(UI::UIManager, getElements),

			"deleteElement", sol::overload(
				static_cast<void(UI::UIManager::*)(uint32_t)>(&UI::UIManager::deleteElement),
				static_cast<void(UI::UIManager::*)(UI::UIElement*)>(&UI::UIManager::deleteElement)
			),

			"setDefaultTextGlyph", sol::overload(
				static_cast<void(UI::UIManager::*)(uint32_t)>(&UI::UIManager::setDefaultTextGlyph),
				static_cast<void(UI::UIManager::*)(Texture*)>(&UI::UIManager::setDefaultTextGlyph)
			),

			BIND(UI::UIManager, getDefaultTextGlyph)
		);


		// Scripts
		lua.new_usertype<Script>("Script",
			sol::no_constructor,

			BIND(Script, meta),

			BIND(Script, getId),
			BIND(Script, getScene),

			BIND(Script, isEnabled),
			BIND(Script, setEnabled),

			BIND(Script, execute)
		);

		lua.new_usertype<ScriptsManager>("ScriptsManager",
			sol::no_constructor,

			"createScript", sol::overload(
				[](ScriptsManager& self) {
					return self.createScript();
				},

				[](ScriptsManager& self, const char* script) {
					return self.createScript(script);
				},

				[](ScriptsManager& self, const char* script, uint32_t id) {
					return self.createScript(script, id);
				},

				[](ScriptsManager& self, const char* script, uint32_t id, bool override) {
					return self.createScript(script, id, override);
				},

				static_cast<Script * (ScriptsManager::*)(const char*, uint32_t, bool)>(&ScriptsManager::createScript),


				[](ScriptsManager& self, Scene* scene) {
					return self.createScript(scene);
				},

				[](ScriptsManager& self, Scene* scene, const char* script) {
					return self.createScript(scene, script);
				},

				[](ScriptsManager& self, Scene* scene, const char* script, uint32_t id) {
					return self.createScript(scene, script, id);
				},

				[](ScriptsManager& self, Scene* scene, const char* script, uint32_t id, bool override) {
					return self.createScript(scene, script, id, override);
				},

				static_cast<Script * (ScriptsManager::*)(Scene*, const char*, uint32_t, bool)>(&ScriptsManager::createScript)
			),

			"deleteScript", sol::overload(
				static_cast<void(ScriptsManager::*)(uint32_t)>(&ScriptsManager::deleteScript),
				static_cast<void(ScriptsManager::*)(Script*)>(&ScriptsManager::deleteScript)
			),

			BIND(ScriptsManager, getScriptsCount),
			BIND(ScriptsManager, getScripts),

			BIND(ScriptsManager, getScript),

			BIND(ScriptsManager, execute)
		);


		// Shaders
		lua.new_enum<SHADER_TYPE>("SHADER_TYPE", {
			{ "NONE", SHADER_TYPE::NONE },
			{ "WORLD", SHADER_TYPE::WORLD },
			{ "SKYBOX", SHADER_TYPE::SKYBOX },
			{ "UI", SHADER_TYPE::UI },
			});

		lua.new_usertype<ShaderSource>("ShaderSource",
			sol::constructors<
			ShaderSource()
			>(),

			BIND(ShaderSource, vertex),
			BIND(ShaderSource, fragment)
		);

		lua.new_usertype<Shader>("Shader",
			sol::no_constructor,

			BIND(Shader, getType),

			BIND(Shader, setSource),
			BIND(Shader, activate),

			BIND(Shader, setMat3),
			BIND(Shader, setMat4),

			BIND(Shader, setFloat),
			BIND(Shader, setInt),

			BIND(Shader, setIVec2),

			BIND(Shader, setVec2),
			BIND(Shader, setVec3),
			BIND(Shader, setVec4)
		);

		lua.new_usertype<ShadersManager>("ShadersManager",
			sol::no_constructor,

			BIND(ShadersManager, getShader),

			BIND(ShadersManager, getActive),

			"setActive", sol::overload(
				static_cast<void(ShadersManager::*)(Shader*)>(&ShadersManager::setActive),
				static_cast<void(ShadersManager::*)(const SHADER_TYPE&)>(&ShadersManager::setActive)
			)
		);


		// Components
		lua.new_enum<COMPONENT_TYPE>("COMPONENT_TYPE", {
			{ "NONE", COMPONENT_TYPE::NONE },
			{ "MESH", COMPONENT_TYPE::MESH },
			{ "POINT_LIGHT", COMPONENT_TYPE::POINT_LIGHT },
			{ "DIRECTIONAL_LIGHT", COMPONENT_TYPE::DIRECTIONAL_LIGHT },
			{ "BOX_COLLIDER", COMPONENT_TYPE::BOX_COLLIDER },
			{ "RIGID_BODY", COMPONENT_TYPE::RIGID_BODY },
			});

		lua.new_enum<Components::COLLIDER_TYPE>("COLLIDER_TYPE", {
			{ "NONE", Components::COLLIDER_TYPE::NONE },
			{ "BOX", Components::COLLIDER_TYPE::BOX },
			});

		lua.new_enum<Components::LIGHT_TYPE>("LIGHT_TYPE", {
			{ "NONE", Components::LIGHT_TYPE::NONE },
			{ "POINT", Components::LIGHT_TYPE::POINT },
			{ "DIRECTIONAL", Components::LIGHT_TYPE::DIRECTIONAL }
			});

		lua.new_usertype<Components::Component>("Component",
			sol::no_constructor,

			BIND(Components::Component, getType),

			BIND(Components::Component, getGameObject)
		);

		lua.new_usertype<Components::Mesh>("Mesh",
			sol::no_constructor,
			sol::base_classes, sol::bases<Components::Component>(),

			"setModel", sol::overload(
				static_cast<void(Components::Mesh::*)(Model*)>(&Components::Mesh::setModel),
				static_cast<void(Components::Mesh::*)(uint32_t)>(&Components::Mesh::setModel)
			),

			"setTexture", sol::overload(
				static_cast<void(Components::Mesh::*)(Texture*)>(&Components::Mesh::setTexture),
				static_cast<void(Components::Mesh::*)(uint32_t)>(&Components::Mesh::setTexture)
			),

			BIND(Components::Mesh, getModel),
			BIND(Components::Mesh, getTexture),

			BIND(Components::Mesh, setColor),
			BIND(Components::Mesh, getColor)
		);

		lua.new_usertype<Components::Light>("Light",
			sol::no_constructor,
			sol::base_classes, sol::bases<Components::Component>(),

			BIND(Components::Light, getLightType),

			BIND(Components::Light, setColor),
			BIND(Components::Light, getColor),

			BIND(Components::Light, setIntensity),
			BIND(Components::Light, getIntensity)
		);

		lua.new_usertype<Components::PointLight>("PointLight",
			sol::no_constructor,
			sol::base_classes, sol::bases<Components::Light>(),

			BIND(Components::PointLight, setDistance),
			BIND(Components::PointLight, getDistance)
		);

		lua.new_usertype<Components::DirectionalLight>("DirectionalLight",
			sol::no_constructor,
			sol::base_classes, sol::bases<Components::Light>()
		);

		lua.new_usertype<Components::RigidBody>("RigidBody",
			sol::no_constructor,
			sol::base_classes, sol::bases<Components::Component>(),

			BIND(Components::RigidBody, setMass),
			BIND(Components::RigidBody, getMass),

			BIND(Components::RigidBody, setVelocity),
			BIND(Components::RigidBody, getVelocity),

			BIND(Components::RigidBody, setMomentum),
			BIND(Components::RigidBody, getMomentum),

			BIND(Components::RigidBody, setElasticity),
			BIND(Components::RigidBody, getElasticity),

			BIND(Components::RigidBody, setCanBounce),
			BIND(Components::RigidBody, canBounce),

			BIND(Components::RigidBody, applyImpulseForce),
			BIND(Components::RigidBody, applyForce),

			BIND(Components::RigidBody, getForces)
		);

		lua.new_usertype<Components::Collider>("Collider",
			sol::no_constructor,
			sol::base_classes, sol::bases<Components::Component>(),

			BIND(Components::Collider, getColliderType)
		);

		lua.new_usertype<Components::BoxCollider>("BoxCollider",
			sol::no_constructor,
			sol::base_classes, sol::bases<Components::Collider>(),

			BIND(Components::BoxCollider, setSize),
			BIND(Components::BoxCollider, getSize),

			BIND(Components::BoxCollider, setOffset),
			BIND(Components::BoxCollider, getOffset)
		);
	}
}
