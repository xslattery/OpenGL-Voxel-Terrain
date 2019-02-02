#version 330

uniform vec3 lightDirection;
uniform vec3 lightColor;
uniform float lightIntencity;

in vec4 fColor;
in vec3 norm;

out vec4 Color;

vec3 lighting() {
	
	vec4 usingColor;
	usingColor = fColor;

	float ambientStrength = 0.3f;
    vec3 ambient = ambientStrength * lightColor;

	vec3 tmp_norm = normalize( norm );
	vec3 lightDir = -normalize( lightDirection );

	float diff = max(dot(norm, lightDir), 0.0);
	vec3 diffuse = diff * lightColor;

	vec3 result = ( ambient + diffuse ) * usingColor.xyz * lightIntencity;

	return result;
}

void main() {
	Color = vec4( lighting(), 1.0f );
}