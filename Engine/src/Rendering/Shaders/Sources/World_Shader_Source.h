#pragma once
#include <Yngin/Rendering/Shaders.h>

namespace Yngin {
	namespace ShaderSources {
		const ShaderSource world = {
			// vertex
			R"(
#version 460 core

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec2 inTexCoord;
layout(location = 2) in vec3 inNormal;
layout(location = 3) in uint inMatId;

uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;
uniform mat3 normalMatrix;

out vec3 fPosition;
out vec2 fTexCoord;
out vec3 fNormal;
flat out uint fMatId;

void main() {
	vec4 worldPosition = model * vec4(inPosition, 1.0);

	gl_Position = projection * view * worldPosition;

	fPosition = worldPosition.xyz;
	fTexCoord = inTexCoord;
	fNormal = normalize(normalMatrix * inNormal);
	fMatId = inMatId;
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
	float intensity;
};

struct Material {
	vec3 ambientColor;
	vec3 diffuseColor;
	vec3 specularColor;
	float specularComponent;
};

struct SceneSettings {
	vec3 ambientLight;
};

in vec3 fPosition;
in vec2 fTexCoord;
in vec3 fNormal;
flat in uint fMatId;

out vec4 FragColor;

uniform bool isLight;
uniform int lightsCount;
uniform Light lights[MAX_LIGHTS];
uniform Material materials[256];

uniform vec3 cameraPos;

uniform SceneSettings scene;

uniform sampler2D tex0;
uniform vec4 color;

void main() {
	vec3 totalLight;

	vec3 diffusion;
	vec3 specular;

	Material mat = materials[fMatId];
	
	if (!isLight) {
		for (int i = 0; i < lightsCount; i++) {
			Light l = lights[i];
			vec3 diff = l.position - fPosition;
			float dist = length(diff);
			vec3 lightDir = diff / (dist + 0.0001);

			float dotProduct = max(dot(fNormal, lightDir), 0.0);
			
			float distRatio = clamp(dist / l.distance, 0.0, 1.0);
			float attenuation = 1.0 - (distRatio * distRatio);
			attenuation *= attenuation;

			diffusion += dotProduct * l.intensity * attenuation * l.color;
			
			vec3 viewDir = normalize(cameraPos - fPosition);
			vec3 reflectDir = reflect(-lightDir, fNormal);
			
			specular += pow(max(dot(viewDir, reflectDir), 0.0), mat.specularComponent) * l.color * l.intensity * 0.5;
		}

		totalLight = mat.diffuseColor * mat.ambientColor + diffusion * mat.diffuseColor + specular * mat.specularColor;
		totalLight = clamp(totalLight / (totalLight + vec3(1.0f)), vec3(0.0), vec3(1.0));
	} else {
		totalLight = mat.diffuseColor;
	}
	
	FragColor = texture(tex0, fTexCoord) * color * vec4(totalLight, 1.0);
}
			)",
		};
	}
}
