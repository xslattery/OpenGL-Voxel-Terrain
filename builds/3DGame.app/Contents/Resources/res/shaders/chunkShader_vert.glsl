#version 330

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec4 color;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

out vec4 fColor;
out float visibility;

const float density = 0.003;
const float gradient = 3;

void main () {
    gl_Position = projection * view * model * vec4(position, 1.0);
    fColor = color;

    float distance = length( (view * model * vec4(position, 1.0)).xyz );
    visibility = exp( -pow(distance*density, gradient) );
    visibility = clamp( visibility, 0.0, 1.0 );
}

