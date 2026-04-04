#pragma once
#include <Yngin/Rendering/Shaders.h>

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
	vec2 localTc = vec2(
		fTexCoord.x * 4 - floor(fTexCoord.x * 4),
		fTexCoord.y * 3 - floor(fTexCoord.y * 3)
	);
	
	vec2 tiles = vec2(4.0, 3.0);
	vec2 tile = vec2(
		floor(fTexCoord.x * 4),
		floor(fTexCoord.y * 3)
	);
	
	vec2 texel = 2.0 / (vec2(textureSize(tex0, 0)) / tiles);

	localTc = clamp(localTc, texel, 1.0 - texel);
	
	vec2 tc = localTc / tiles + tile / tiles;

	FragColor = texture(tex0, tc);
}
			)",
		};
	}
}
