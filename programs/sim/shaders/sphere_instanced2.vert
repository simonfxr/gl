
uniform mat4 pMatrix;

in vec4 position;
in vec3 normal;
in vec4 colorShininess;
in mat4 mvMatrix;

out vec3 ecPosition;
out vec3 ecNormal;
out vec3 color;
out float shininess;

void main() {
    vec4 ecPos4 = mvMatrix * position;
    ecPosition = vec3(ecPos4);
    ecNormal = normalize((mvMatrix * vec4(normal, 0.)).xyz);
    gl_Position = pMatrix * ecPos4;
    color = colorShininess.rgb;
    shininess = colorShininess.a;
}
