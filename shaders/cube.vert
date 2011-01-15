#version 330

uniform mat4 mvp;

in vec3 vertex;

out vec4 color;

void main() {
     gl_Position = mvp * vec4(vertex, 1);
     color = vec4(vertex * 0.5 + vec3(0.5), 1);
}
