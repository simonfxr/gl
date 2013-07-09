
#include "gamma.h"
#include "simplex_noise2.h"
#include "constants.h"

uniform float time;
uniform mat3 transform;

const bool SPLIT_SCREEN = false;
const bool APPLY_TRANSFORM = false;

in vec2 worldPosition;
out vec4 color;

float stretch(float x) {
    return x;
    float x1 = 1 - x;
    return 1 - smoothstep(0, 1, x1);
}

float evalCrystal(vec2 p) {
    const int n = 7;
    float phi = 0;
    float omega = 2 * 3.14159 / n;
    float a = float(n);
    for (int i = 0; i < n; ++i) {
        a += cos(cos(phi) * p.x + sin(phi) * p.y + time * 0.2);
        phi += omega;
    }

    a *= 0.5;
    a = fract(a);
    return a;
}

vec2 polar(vec2 v) {
    vec2 p;
    p.x = length(v);

    if (p.x < 1e-4f) {
        p.y = 0;
    } else if (v.x < 0 && abs(v.y) < 1e-4f) {
        p.y = M_PI;
    } else {
        p.y = atan(v.y, p.x + v.x) * 2;
    }

    return p;
}

vec2 kart(vec2 p) {
    return p.x * vec2(cos(p.y), sin(p.y));
}

vec3 eval(vec2 p0) {
    vec2 p1 = p0 + (1 * time) * normalize(vec2(1, 2));
    vec2 p2 = p0;
    
    vec2 f;
    float tau = time * 0.05;
    float omega = 0.02 * sin(tau) + 0.01 * cos(2 * tau);
    f.x = snoise(p1 * omega + vec2(39, 113)) + snoise(p1 * omega * 2 + vec2(13, 27)) * 0.5;
    f.y = snoise(p1 * omega + vec2(19, 111)) + snoise(p1 * omega * 2 + vec2(-7, 3)) * 0.5;
    
    /* f = normalize(f); */

    /* f *= snoise(p * 1.55 + vec2(-17, 33)); */

    vec2 pol = polar(p2);
    float r = pol.x;
    float phi = pol.y;
        
//    f *= 2 * (snoise(p * omega * 3 + vec2(31, -11)) + 1);
    float w = 0.5;

    return evalCrystal(p2 * (w * sin(phi) + 1) + f) * vec3(1);

    return length(f) * vec3(1);
}

vec3 evalPolar(vec2 p) {
    p = polar(p);

    float omega = 0.01;
    vec2 f;

    f.x = snoise(p * omega) + snoise(p * omega * 2 + vec2(13, 27)) * 0.5;
    f.y = snoise(p * omega + vec2(19, 111)) + snoise(p * omega * 2 + vec2(-7, 3)) * 0.5;

    p.x *= 2 * M_PI;
    p *= 0.05;
    return evalCrystal(p) * vec3(1);
}

vec2 applyTransform(vec2 v) {
    vec3 p3 = transform * vec3(v, 1);
    return p3.xy / p3.z;
}

vec2 applyTransform2(vec2 v) {
    return applyTransform(v) * 50;
}

void main() {
    vec2 position = worldPosition;
    vec3 albedo = vec3(0);

    if (SPLIT_SCREEN) {
        const float border = 0.01;
        const float border2 = border * 0.5;
        
        if (position.y < - border2) {
            position.y += 0.5;
            position *= 2;
            albedo = evalCrystal(applyTransform2(position)) * vec3(1);
        } else if (position.y > border2) {
            position.y -= 0.5;
            position *= 2;
            albedo = eval(applyTransform2(position));
        } else {
            albedo = vec3(0);
        }
    } else {
        albedo = eval(applyTransform2(position));
    }
    
    albedo = smoothstep(0, 1, albedo);
    color = gammaCorrect(vec4(albedo, 1));
}
