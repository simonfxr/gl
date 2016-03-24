
uniform float time;
uniform mat3 transform;

in vec2 worldPosition;

flat in vec2 C;
flat in float zoom;
flat in vec2 shift;

out vec4 fragColor;

const float pi = 3.14159265;

vec2 cmul(vec2 a, vec2 b) {
    return vec2(a.x * b.x - a.y * b.y, a.x * b.y + a.y * b.x);
}

float sinShift(float a, float t) {
    return a * sin(t) + (1 - a);
}

float evalFractal(vec2 x) {
    x += shift;
    x *= zoom;
    
    const float L = 1000;
    int i = 0;
    int N = 200;

    while (i < N && dot(x, x) < L * L) {
        x = cmul(x, x) + C;
        ++i;
    }

    float h = float(i) / float(N);
    h = 1 - pow(1 - h, 3);
    return h;
}

float sigmoid(float t) {
    return 1/(1 + exp(-t));
}

vec3 sphericToEuclidean(vec3 p) {
    float r = p.x;
    float theta = p.y;
    float phi = p.z;
        
    float sp = sin(phi);
    float cp = cos(phi);
    float st = sin(theta);
    float ct = cos(theta);

    vec3 xyz;
    xyz.x = st * cp;
    xyz.y = st * sp;
    xyz.z = ct;
    xyz *= r;
    return xyz;
}

void main() {
    vec2 p = (transform * vec3(worldPosition, 1)).xy;
    float a = evalFractal(p);

    /* if (a < 0.4) */
    /*     fragColor = vec4(vec3(0), 1); */
    /* else */
    /*     fragColor = vec4(vec3(1), 1); */
    /* return; */

    float phi = pow(a, 1/1.5) * pi/2;
    float theta = pow(clamp(pow(a, 1/1.11) - 0.4, 0, 1), 1/1.4) * pi/2 + pi/4;
    float r = sigmoid(a * 4);

    fragColor = vec4(sphericToEuclidean(vec3(r, theta, phi)), 1);
//    fragColor = vec4(vec3(a), 1);
}
