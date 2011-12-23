
in vec3 ecNormal;

out vec4 color;

void main() {
    vec3 normal = normalize(ecNormal);
    vec3 baseColor = vec3(0.6);
    vec3 shadedColor = (dot(normal, baseColor) / dot(baseColor, baseColor)) * baseColor;
    color = vec4(0.4 * baseColor + 0.6 * shadedColor, 1);
}
