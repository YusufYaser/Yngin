#pragma once
#include <Yngin/Renderer/Shaders.h>

namespace Yngin {
	namespace ShaderSources {
		const ShaderSource skybox = {
			// vertex
			R"(
#version 460 core

layout(location = 0) in vec3 inPos;
layout(location = 1) in vec2 inTexCoord;

uniform mat4 projection;
uniform mat4 view;

out vec2 fTexCoord;

void main() {
	gl_Position = projection * view * vec4(inPos, 1.0);

	fTexCoord = inTexCoord;
}
			)",

			// fragment
			R"(
#version 460 core

in vec2 fTexCoord;

out vec4 FragColor;

uniform sampler2D tex0;

void main() {
	FragColor = texture(tex0, fTexCoord);
}
			)",
		};
	}
}
