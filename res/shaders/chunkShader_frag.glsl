#version 330

uniform vec4 skyColor;

in vec4 fColor;
in float visibility;

out vec4 Color;

void main() {
	Color = fColor;
	// Color = mix( skyColor , Color, visibility);
}