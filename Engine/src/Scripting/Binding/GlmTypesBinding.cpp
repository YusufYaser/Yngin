#include "../Scripting_Internal.h"
#include <glm/glm.hpp>

namespace Yngin {
	void ScriptsManager::Impl::bindGlmTypes() {
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
	}
}
