
uniform vec3 ecLight;
uniform vec3 albedo;

in vec3 ecPosition;
in vec3 ecNormal;

out vec4 fragColor;

void main() {

    vec3 N = normalize(ecNormal);
    vec3 V = normalize(- ecPosition);
    vec3 L = normalize(ecLight - ecPosition);

    /* float diff = dot(N, L); */
    /* float spec = pow(dot(V, reflect(-L, N)), 30); */

    /* vec3 color = (0.4 + 0.4 * diff) * albedo + spec * 0.2 * vec3(1); */
    /* fragColor = vec4(color, 1); */

    vec3 color = 0.4 * vec3(1);
    color += max(0, dot(N, L)) * vec3(1);
    fragColor = vec4(color, 1);
}
