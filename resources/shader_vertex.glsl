#version 330 core
layout(location = 0) in vec4 vertPos;
layout(location = 1) in vec3 vertNor;
layout(location = 2) in vec2 vertTex;

uniform mat4 P;
uniform mat4 V;
uniform mat4 M;
uniform vec3 lightPos;


out vec3 vertex_pos;
out vec3 vertex_normal;
out vec2 vertex_tex;

out vec3 fragNor;
out vec3 lightDir;

void main() {
	vertex_normal = vec4(M * vec4(vertNor, 0.0)).xyz;
	vec4 pos = M * vertPos;
	gl_Position = P * V * pos;
	vertex_tex = vertTex;
	vertex_pos = pos.xyz;

	fragNor = (V * M * vec4(vertNor, 0.0)).xyz;
	lightDir = vec3(V * (vec4(lightPos - (M * vertPos).xyz, 0.0)));
}