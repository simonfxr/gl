
uniform vec3 ecLightDir;
uniform vec3 albedo;

in vec3 ecNormal;

out vec4 fragColor;

void
main()
{
    vec3 N = normalize(ecNormal);

    float diff = max(-dot(N, ecLightDir), 0);

    fragColor.a = 1;
    fragColor.rgb = (0.8 * diff + 0.4) * albedo;
}
