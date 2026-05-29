R"(
#version 460 core

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec2 inTexCoord;
layout(location = 2) in vec3 inNormal;
layout(location = 3) in uint inMatId;

struct InstanceVertexOffset {
	mat4 model;
	mat4 normalMatrix;
};

layout(std430, binding = 0) buffer InstanceVertexOffsets {
	InstanceVertexOffset vertexOffsets[];
};

uniform mat4 viewProjection;
uniform bool instancing;
uniform mat4 uModel;
uniform mat3 uNormalMatrix;

out flat int fInstanceID;
out flat int fInstancing;
out vec3 fPosition;
out vec2 fTexCoord;
out vec3 fNormal;

void main() {
	int id = gl_InstanceID;
	
	mat4 model = uModel;
	mat3 normalMatrix = uNormalMatrix;
	
	if (instancing) {
		model = vertexOffsets[id].model;
		normalMatrix = mat3(vertexOffsets[id].normalMatrix);
	}
	
	vec4 worldPosition = model * vec4(inPosition, 1.0);

	gl_Position = viewProjection * worldPosition;

	fInstanceID = id;
	fInstancing = int(instancing);
	fPosition = worldPosition.xyz;
	fTexCoord = inTexCoord;
	fNormal = normalize(normalMatrix * inNormal);
}
)"
