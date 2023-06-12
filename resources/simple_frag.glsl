#version 330 core 

out vec4 color;

uniform vec3 MatAmb;
uniform vec3 MatDif;
uniform vec3 MatSpec;
uniform float MatShine;
uniform int flip;

//interpolated normal and light vector in camera space
in vec3 fragNor;
in vec3 lightDir;
//position of the vertex in camera space
in vec3 EPos;


void main()
{
	vec3 normal = normalize(fragNor);
	if (flip == 1) {
		normal = normal * -1.0f;
	}

	vec3 fragMatAmb = MatAmb;
	vec3 fragMatDif = MatDif;
	vec3 fragMatSpec = MatSpec;
	float fragMatShine = MatShine;

	vec3 light = normalize(lightDir);
	float dC = max(0, dot(normal, light));
	vec3 halfV = normalize(-1*EPos) + normalize(light);
	float sC = pow(max(dot(normalize(halfV), normal), 0), fragMatShine);

	color = vec4(fragMatAmb + dC*fragMatDif + sC*fragMatSpec, 1.0);
}