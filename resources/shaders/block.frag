#version 330 core

out vec4 FragColor;

in vec3 Pos;
in vec2 TexCoords;
in float LightValue;
in float vertexAO;

//in vec3 cameraPos;

uniform sampler2D textureAtlas;

void main()
{
	vec4 vertexColor = texture(textureAtlas, TexCoords);

	FragColor = LightValue * (texture(textureAtlas, TexCoords) * clamp(0.225 * vertexAO + 0.3, 0.0f, 1.0f));
}