R"(
#version 460 core

struct UITransform {
	float xScale;
	int xOffset;
	float yScale;
	int yOffset;
};

layout(location = 0) in vec3 inPos;
layout(location = 1) in vec2 inTexCoord;

uniform UITransform uiPosition;
uniform UITransform uiSize;
uniform ivec2 screenSize;

out vec2 fTexCoord;

void main() {
	gl_Position = vec4(
		(inPos.x * (uiSize.xScale + uiSize.xOffset * 1.0f / screenSize.x) * 2) + ((uiPosition.xScale + uiPosition.xOffset * 1.0f / screenSize.x) * 2 - 1),
		(inPos.y * (uiSize.yScale + uiSize.yOffset * 1.0f / screenSize.y) * 2) - ((uiPosition.yScale + uiPosition.yOffset * 1.0f / screenSize.y) * 2 - 1),
		0.0,
		1.0
	);

	fTexCoord = inTexCoord;
}
)"
