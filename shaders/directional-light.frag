#include "directional-light.h"

vec4 DirectionalLight(vec3 position, vec3 normal, vec3 lightDir,
                      vec4 intensity_diff, vec4 intensity_spec, float shininess) {

    vec3 l = lightDir;
    vec3 v = normalize(-position);
    vec3 h = normalize(v + l);
    float cosTh = max(0.0, dot(normal, h));
    float cosTi = max(0.0, dot(normal, l));
    vec4 radiance = (intensity_diff + intensity_spec * pow(cosTh, shininess)) * cosTi;
    return radiance;
}
