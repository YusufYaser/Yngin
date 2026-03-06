#pragma once
#include <Yngin/Renderer/Shaders.h>

namespace Yngin {
	namespace ShaderSources {
		const ShaderSource world = {
			// vertex
			R"(
#version 460 core

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec2 inTexCoord;
layout(location = 2) in vec3 inNormal;

uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;
uniform mat3 normalMatrix;

out vec3 fPosition;
out vec2 fTexCoord;
out vec3 fNormal;

void main() {
	vec4 worldPosition = model * vec4(inPosition, 1.0);

	gl_Position = projection * view * worldPosition;

	fPosition = worldPosition.xyz;
	fTexCoord = inTexCoord;
	fNormal = normalize(normalMatrix * inNormal);
}
			)",

			// fragment
			R"(
#version 460 core

#define MAX_LIGHTS 32

struct Light {
	vec3 position;
	vec3 color;
	float distance;
};

in vec3 fPosition;
in vec2 fTexCoord;
in vec3 fNormal;

out vec4 FragColor;

uniform int lightsCount;
uniform Light lights[MAX_LIGHTS];

uniform bool isLight;

uniform sampler2D tex0;
uniform vec4 color;

void main() {
	vec3 totalLightColor = vec3(0.1, 0.1, 0.1);
	if (!isLight) {
		for (int i = 0; i < lightsCount; i++) {
			vec3 diff = lights[i].position - fPosition;
			float dist = length(diff);
			vec3 dir = diff / (dist + 0.0001);

			float dotProduct = max(dot(fNormal, dir), 0.0);

			float intensity = clamp(1.0 - (dist / lights[i].distance), 0.0, 1.0);
			intensity *= intensity;

			totalLightColor += dotProduct * intensity * lights[i].color;
		}

		totalLightColor = clamp(totalLightColor + vec3(0.1), vec3(0.0), vec3(1.0));
	} else {
		totalLightColor = vec3(1.0);
	}

	FragColor = texture(tex0, fTexCoord) * color * vec4(totalLightColor, 1.0);
}
			)",
		};
	}
}
