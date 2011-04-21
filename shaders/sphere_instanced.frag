#version 330

const vec3 COLOR = vec3(0.55, 0.55, 0);

in vec3 ecPosition;
in vec3 ecNormal;

out vec4 fragColor;

void main() {
    vec3 n = normalize(ecNormal);
    fragColor = vec4(dot(COLOR, ecNormal) * COLOR, 1);
}
