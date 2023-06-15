#version 330 core
in vec3 vertex_normal;
in vec3 vertex_pos;
in vec2 vertex_tex;

uniform sampler2D tex;

uniform vec3 MatAmb;
uniform vec3 MatDif;
uniform vec3 MatSpec;
uniform float MatShine;

out vec4 fragCol;
in vec3 fragNor;
in vec3 lightDir;
in vec3 EPos;


void main() {
	vec3 normal = normalize(fragNor);

	vec3 fragMatAmb = MatAmb;
	vec3 fragMatDif = MatDif;
	vec3 fragMatSpec = MatSpec;
	float fragMatShine = MatShine;

	vec3 n = normalize(vertex_normal);
	vec3 lp = vec3(10, -20, -100);
	vec3 ld = normalize(vertex_pos - lp);
	float diffuse = dot(n, ld);

	if(MatShine != 5.0) {
		vec4 tcol = texture(tex, vertex_tex);

		vec3 light = normalize(lightDir);
		float dC = max(0, dot(normal, light));
		fragCol = vec4(tcol.xyz * dC, 1.0);
	} else {
		vec3 light = normalize(lightDir);
		float dC = max(0, dot(normal, light));
		vec3 halfV = normalize(-1 * EPos) + normalize(light);
		float sC = pow(max(dot(normalize(halfV), normal), 0), fragMatShine);

		fragCol = vec4(fragMatAmb + dC * fragMatDif + sC * fragMatSpec, 1.0);
	}
}
