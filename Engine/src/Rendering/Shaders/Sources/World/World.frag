R"(
#version 460 core

#define MAX_POINT_LIGHTS 256
#define MAX_DIRECTIONAL_LIGHTS 2

layout(location = 0) out vec4 FragColor;
layout(location = 1) out uint outID;

struct InstanceOffsetMaterial {
	vec3 ambientColor;
	float _pad1;
	vec3 diffuseColor;
	float _pad2;
	vec3 specularColor;
	float _pad3;
	float specularComponent;
    float _pads[3];
};

struct InstanceFragmentOffset {
	vec4 color;
	InstanceOffsetMaterial material;
	uint objectId;
	int isLight;
    int _pads[2];
};

struct PointLight {
	vec3 position;
	float _pad1;
	vec3 color;
	float _pad2;
	float distance;
	float intensity;
	float _pads[2];
};

struct DirectionalLight {
	int index;
	int _pads[3];
	mat4 viewProjection;
	vec3 color;
	float _pad2;
	vec3 direction;
	float intensity;
};

layout(std430, binding = 1) buffer InstanceFragmentOffsets {
	InstanceFragmentOffset fragOffsets[];
};

layout(std430, binding = 2) buffer SceneLights {
	int pointLightsCount;
	int directionalLightsCount;
	int _pads[2];
	PointLight pointLights[MAX_POINT_LIGHTS];
	DirectionalLight directionalLights[MAX_DIRECTIONAL_LIGHTS];
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

in flat int fInstanceID;
in flat int fInstancing;
in vec3 fPosition;
in vec2 fTexCoord;
in vec3 fNormal;

uniform bool uIsLight;
uniform Material uMaterial;

uniform vec3 cameraPos;

uniform SceneSettings scene;

uniform sampler2D tex;
uniform sampler2DArrayShadow shadowMaps;
uniform vec4 uColor;

float CalculateDirectionalLightShadow(DirectionalLight l) {
	vec4 lightSpace = l.viewProjection * vec4(fPosition, 1.0); 
	vec3 projCoords = lightSpace.xyz / lightSpace.w;
	
	vec3 shadowCoord = projCoords * 0.5 + 0.5;

	float currentDepth = shadowCoord.z;
	
	float shadows = 0.0;

	vec2 texelSize = 1.0 / vec2(textureSize(shadowMaps, 0).xy);

	for (int x = -1; x <= 1; x++) {
		for (int y = -1; y <= 1; y++) {
			shadows += 1.0 - texture(
				shadowMaps,
				vec4(
					shadowCoord.xy + vec2(x,y) * texelSize * 2.0,
					l.index,
					currentDepth - 0.001
				)
			);
		}
	}
	
	return shadows / 9.0;
}

void main() {
	int id = fInstanceID;

	vec3 totalLight;

	vec3 diffusion;
	vec3 specular;
	
	Material mat = uMaterial;
	bool isLight = uIsLight;
	vec4 color = uColor;

	if (fInstancing != 0) {
		InstanceOffsetMaterial omat = fragOffsets[fInstanceID].material;
		mat.ambientColor = omat.ambientColor;
		mat.diffuseColor = omat.diffuseColor;
		mat.specularColor = omat.specularColor;
		mat.specularComponent = omat.specularComponent;
		isLight = fragOffsets[fInstanceID].isLight != 0;
		color = fragOffsets[fInstanceID].color;
	}
	
	if (!isLight) {
		vec3 normal = normalize(fNormal);
		vec3 viewDir = normalize(cameraPos - fPosition);

		for (int i = 0; i < pointLightsCount; i++) {
			PointLight l = pointLights[i];
			vec3 diff = l.position - fPosition;
			float dist = length(diff);
			vec3 lightDir = diff / (dist + 0.0001);

			float dotProduct = max(dot(normal, lightDir), 0.0);
			
			float distRatio = clamp(dist / l.distance, 0.0, 1.0);
			float attenuation = 1.0 - (distRatio * distRatio);
			attenuation *= attenuation;

			diffusion += dotProduct * l.intensity * attenuation * l.color;
			
			vec3 reflectDir = reflect(-lightDir, normal);
			if (dotProduct > 0.0) {
				specular += pow(max(dot(viewDir, reflectDir), 0.0), mat.specularComponent) * l.color * l.intensity * 0.5 * attenuation;
			}
		}

		for (int i = 0; i < directionalLightsCount; i++) {
			DirectionalLight l = directionalLights[i];
			vec3 lightDir = normalize(-l.direction);

			float dotProduct = max(dot(normal, lightDir), 0.0);

			float shadow = CalculateDirectionalLightShadow(l);
			float visibility = 1.0 - shadow;

			diffusion += dotProduct * l.intensity * l.color * visibility;
			
			vec3 reflectDir = reflect(-lightDir, normal);
			if (dotProduct > 0.0) {
				specular += pow(max(dot(viewDir, reflectDir), 0.0), mat.specularComponent) * l.color * l.intensity * 0.5 * visibility;
			}
		}

		totalLight = mat.diffuseColor * mat.ambientColor + diffusion * mat.diffuseColor + specular * mat.specularColor;
		totalLight = clamp(totalLight / (totalLight + vec3(1.0)), vec3(0.0), vec3(1.0));
	} else {
		totalLight = mat.diffuseColor;
	}
	
	FragColor = texture(tex, fTexCoord) * color * vec4(totalLight, 1.0);
	outID = fragOffsets[fInstanceID].objectId;
}
)"
