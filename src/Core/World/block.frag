#version 330 core

out vec4 FragColor;

in vec3 Pos;
in vec2 TexCoords;
in float LightValue;

//in vec3 cameraPos;

uniform sampler2D textureAtlas;

void main()
{
	//FragColor = vec4(diffuse + ambientColor, 1.0f);
	FragColor = LightValue * texture(textureAtlas, TexCoords);
}