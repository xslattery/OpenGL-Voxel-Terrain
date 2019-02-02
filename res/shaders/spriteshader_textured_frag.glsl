#version 330

in vec2 TexCoord;

uniform sampler2D ourTexture;

out vec4 Color;

void main() {
    Color = texture(ourTexture, TexCoord);
}
