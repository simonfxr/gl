
uniform float time;
uniform mat4 worldMatrix;

const int ITER = 11;

const float RAD = 0.9;

const vec4 C0 = 0.81 * normalize(vec4(-0.4, 0.4, 1.2, 1.1));

const float W = 0.5;

in vec3 vTexCoord;
out float fValue;

vec4
mulQ(vec4 a, vec4 b)
{
    return vec4(a.x * b.x - a.y * b.y - a.z * b.z - a.w * b.w,
                a.x * b.y + a.y * b.x + a.z * b.w - a.w * b.z,
                a.x * b.z - a.y * b.w + a.z * b.x + a.w * b.y,
                a.x * b.w + a.y * b.z - a.z * b.y + a.w * b.x);
}

vec4
square(vec4 z)
{
    return mulQ(z, z);
}

float
J(vec4 z, vec4 c)
{
    for (int i = 0; i < ITER; ++i)
        z = square(z) + c;
    return length(z);
}

float
world(vec3 p)
{

    float w = sin(time * 0.3) * 0.6;
    float u = cos(time * 0.3 + 1.5) * 0.6;
    vec4 c = C0;
    p *= 0.5;
    vec4 point = 2.1 * vec4(p, w);
    return (J(point, c) - RAD) / RAD;
}

void
main()
{
    vec4 p4 = worldMatrix * vec4(vTexCoord, 1);
    fValue = world(p4.xyz / p4.w);
}
