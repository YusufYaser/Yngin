#include <Yngin/Yngin.h>
#include "Scripting_Internal.h"
#include <glm/glm.hpp>

#define BIND(CLASS, NAME) #NAME, &CLASS::NAME

namespace Yngin {
	void ScriptsManager::Impl::bindTypes() {
		// Types
		lua.new_usertype<glm::vec2>("Vec2",
			sol::constructors<
			glm::vec2(),
			glm::vec2(float),
			glm::vec2(float, float)
			>(),

			BIND(glm::vec2, x),
			BIND(glm::vec2, y)
		);

		lua.new_usertype<glm::vec3>("Vec3",
			sol::constructors<
			glm::vec3(),
			glm::vec3(float),
			glm::vec3(float, float, float),

			glm::vec3(glm::vec2, float)
			>(),

			BIND(glm::vec3, x),
			BIND(glm::vec3, y),
			BIND(glm::vec3, z),

			BIND(glm::vec3, x),
			BIND(glm::vec3, y),
			BIND(glm::vec3, z)
		);

		lua.new_usertype<glm::vec4>("Vec4",
			sol::constructors<
			glm::vec4(),
			glm::vec4(float),
			glm::vec4(float, float, float, float),

			glm::vec4(glm::vec3, float),
			glm::vec4(glm::vec2, float, float)
			>(),

			BIND(glm::vec4, x),
			BIND(glm::vec4, y),
			BIND(glm::vec4, z),
			BIND(glm::vec4, w),

			BIND(glm::vec4, x),
			BIND(glm::vec4, y),
			BIND(glm::vec4, z),
			BIND(glm::vec4, w)
		);

		lua.new_usertype<glm::ivec2>("IVec2",
			sol::constructors<
			glm::ivec2(),
			glm::ivec2(int),
			glm::ivec2(int, int)
			>(),

			BIND(glm::ivec2, x),
			BIND(glm::ivec2, y)
		);

		lua.new_usertype<glm::ivec3>("IVec3",
			sol::constructors<
			glm::ivec3(),
			glm::ivec3(int),
			glm::ivec3(int, int, int),

			glm::vec3(glm::ivec2, int)
			>(),

			BIND(glm::ivec3, x),
			BIND(glm::ivec3, y),
			BIND(glm::ivec3, z),

			BIND(glm::ivec3, x),
			BIND(glm::ivec3, y),
			BIND(glm::ivec3, z)
		);

		lua.new_usertype<glm::ivec4>("IVec4",
			sol::constructors<
			glm::ivec4(),
			glm::ivec4(int),
			glm::ivec4(int, int, int, int),

			glm::ivec4(glm::ivec3, int),
			glm::ivec4(glm::ivec2, int, int)
			>(),

			BIND(glm::ivec4, x),
			BIND(glm::ivec4, y),
			BIND(glm::ivec4, z),
			BIND(glm::ivec4, w),

			BIND(glm::ivec4, x),
			BIND(glm::ivec4, y),
			BIND(glm::ivec4, z),
			BIND(glm::ivec4, w)
		);

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


		lua.new_usertype<Context>("Context",
			sol::no_constructor,

			BIND(Context, getFrame),
			BIND(Context, getMaxFPS),
			BIND(Context, setMaxFPS),

			BIND(Context, getFrameStartTime)
		);

		lua.new_usertype<GameObject>("GameObject",
			sol::no_constructor,

			BIND(GameObject, getId),
			BIND(GameObject, getPosition),
			BIND(GameObject, setPosition),
			BIND(GameObject, getRotation),
			BIND(GameObject, setRotation),
			BIND(GameObject, getScale),
			BIND(GameObject, setScale)
		);

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

		sol::table Yngin = lua.create_table("Yngin");

		Yngin["Context"] = ctx;
		Yngin["InputSystem"] = ctx->getInputSystem();
		Yngin["Window"] = ctx->getWindow();
	}
}