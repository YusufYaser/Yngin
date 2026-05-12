#pragma once
#include <Yngin/Rendering/Shaders.h>

namespace Yngin {
	namespace ShaderSources {
		const ShaderSource depthPrePass = {
			// vertex
			R"(
#version 460 core

layout(location = 0) in vec3 inPosition;

struct InstanceVertexOffset {
	mat4 model;
	mat4 normalMatrix;
};

layout(std430, binding = 0) buffer InstanceVertexOffsets {
	InstanceVertexOffset vertexOffsets[];
};

uniform mat4 viewProjection;

void main() {
	// Depth Pre Pass can only happen when objects are rendered by instancing
	gl_Position = viewProjection * vertexOffsets[gl_InstanceID].model * vec4(inPosition, 1.0);
}
			)",
		};
	}
}
