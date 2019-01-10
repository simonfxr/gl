// #include "point_light2.h"

uniform vec3 spotDirection;
uniform float useSpot;
uniform float spotSmooth;

const float spotOn = 0.96;
const float spotOff = 0.955;

vec4
TeapotLighting(vec3 position,
               vec3 normal,
               vec3 lightPos,
               vec4 intensity_diff,
               vec4 intensity_spec,
               float shininess)
{

    vec3 l = normalize(lightPos - position);

    float intensity = 1;
    if (useSpot > 0) {
        float cosPhi = dot(-l, spotDirection);
        if (spotSmooth <= 0) {
            if (cosPhi < spotOff)
                intensity = 0.;
        } else {
            intensity = 1. - (spotOn - cosPhi) / (spotOn - spotOff);
            intensity = clamp(intensity, 0., 1.);
        }
    }

    vec3 v = normalize(-position);
    vec3 h = normalize(v + l);
    float cosTh = max(0.0, dot(normal, h));
    float cosTi = max(0.0, dot(normal, l));
    vec4 radiance =
      (intensity_diff + intensity_spec * pow(cosTh, shininess)) * cosTi;
    return radiance * intensity;
}
