
const float Ambient = 0.3;
const float Diffuse = 0.9;
const float Specular = 0;

const float BumpDensity = 16;
const float BumpSize = 0.15;

in vec3 tsLight;
in vec3 tsEye;
in vec2 fragUV;

out vec4 fragColor;

vec3 sampleTexture(vec2 uv) {
    ivec2 p = ivec2(floor(BumpDensity * uv));
    if ((p.x + p.y) % 2 != 0)
        return vec3(1, 0, 0);
    else
        return vec3(1);
}

void main() {
    vec2 uv = fragUV;

    vec3 light = normalize(tsLight);
    vec3 eye = normalize(tsEye);

    vec2 dn = fract(BumpDensity * uv) - vec2(0.5);
    float h = dot(dn, dn);
    float idn = inversesqrt(h + 1);
    if (h > BumpSize) {
        dn = vec2(0);
        idn = 1;
    }

    vec3 normal = idn * vec3(dn.x, dn.y, 1);
    /* vec3 normal = vec3(0, 0, 1); */

    float d = clamp(dot(normal, light), 0, 1);
    float s = clamp(dot(eye, reflect(light, normal)), 0, 1);
    s = pow(s, 200);

    vec3 albedo = sampleTexture(uv);
    /* vec3 albedo = vec3(1, 0, 0); // */
    vec3 lightColor = vec3(1, 1, 1);

    fragColor.rgb = (Ambient + d * Diffuse) * albedo + s * Specular * lightColor;
    fragColor.a = 1;
}
