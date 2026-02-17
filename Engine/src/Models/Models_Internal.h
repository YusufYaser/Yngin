#pragma once
#include <Yngin/Yngin.h>

namespace Yngin {
	struct Model::Impl {
		Context* ctx;

		GLuint VAO;
		GLuint VBO;
		GLuint EBO;

		GLsizei indicesCount;
	};
}
