in vec3 ecNormal;
in vec3 ecPosition;

out vec4 fColor;

void
main()
{
    vec3 N = normalize(ecNormal);
    vec3 V = normalize(-ecPosition);
    float diff = max(0, dot(N, V));
    vec3 color = vec3(0.5);
    fColor = vec4(color * (0.2 + 0.8 * diff), 1);
}
