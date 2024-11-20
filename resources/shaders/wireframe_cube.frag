#version 330 core

out vec4 FragColor;

in vec3 Pos;

uniform float time;

void main()
{
	// Calculate angle in radians based on time
    float angle = time;

    float r = 0.5 * (sin(angle + 0.0) + 1.0); // Red
    float g = 0.5 * (sin(angle + (2.0 * 3.14159 / 3.0)) + 1.0); // Green
    float b = 0.5 * (sin(angle + (4.0 * 3.14159 / 3.0)) + 1.0); // Blue

    FragColor = vec4(r, g, b, 1.0f);
}