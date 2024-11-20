#version 330 core

layout(location = 0) in vec3 aPos;
layout(location = 1) in vec2 aTexCoords;
layout(location = 2) in vec2 aTexturePos;
layout(location = 3) in float aLightValue;
layout(location = 4) in float ambientOcclusionValue; // 0 - 3

out vec3 Pos;
out vec2 TexCoords;
out float LightValue;
out float vertexAO;

//out vec3 cameraPos;

uniform mat4 modelMatrix;
uniform mat4 viewMatrix;
uniform mat4 perspectiveMatrix;

float regionWidth = 1.0f / 16;
float regionHeight = 1.0f / 16;

void main()
{

	TexCoords.x = (aTexturePos.x + aTexCoords.x) * regionWidth;
	TexCoords.y = (aTexturePos.y + aTexCoords.y) * regionHeight;

	gl_Position = perspectiveMatrix * viewMatrix * modelMatrix * vec4(aPos, 1.0);

	Pos = aPos;

	LightValue = (aLightValue / 15.0f);
	vertexAO = ambientOcclusionValue;
}
