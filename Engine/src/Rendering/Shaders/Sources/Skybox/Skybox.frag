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
)"
