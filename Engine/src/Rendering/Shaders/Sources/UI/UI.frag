R"(
#version 460 core

struct UICrop {
	vec2 start;
	vec2 end;
};

in vec2 fTexCoord;

out vec4 FragColor;

uniform UICrop uiCrop;

uniform sampler2D tex0;
uniform vec4 color;

void main() {
	vec2 croppedTexCoord;
	croppedTexCoord.x = uiCrop.start.x + fTexCoord.x * (uiCrop.end.x - uiCrop.start.x);
	croppedTexCoord.y = uiCrop.start.y + fTexCoord.y * (uiCrop.end.y - uiCrop.start.y);
	
	FragColor = texture(tex0, croppedTexCoord) * color;
}
)"
