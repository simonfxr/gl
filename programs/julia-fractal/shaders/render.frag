
uniform float time;
uniform mat3 transform;

in vec2 worldPosition;

out vec4 fragColor;

vec2 mul(vec2 a, vec2 b) {
    return vec2(a.x * b.x - a.y * b.y, a.x * b.y + a.y * b.x);
}

float sinShift(float a, float t) {
    return a * sin(t) + (1 - a);
}

float evalFractal(vec2 x) {

    float T = time * 0.3;
    float a = cos(T);
    float b = sin(T);

    mat2 R = mat2(vec2(a, b), vec2(-b, a));

//    vec2 C = sinShift(0.2, 3.1415 * cos(time) + 12) * vec2(0.4, -0.4);
    vec2 C = time * 0.002 * R * vec2(0.4, -0.4);
    
    const float L = 1000;
    int i = 0;
    int N = 200;

    while (i < N && dot(x, x) < L * L) {
        x = mul(x, x) + C;
        ++i;
    }

    float h = float(i) / float(N);
    h = 1 - h;
    h = h * h * h;
    return 1 - h;
}

void main() {
    vec2 p = (transform * vec3(worldPosition, 1)).xy;
    fragColor = vec4(vec3(evalFractal(p)), 1);
}
