#include "point_light.h"
#include "sim_shading.h"

const float Ambient = 0.3;
const float Diffuse = 0.6;
const float Specular = 0.1;

vec4
shade(vec3 ecLight, vec3 ecPosition, vec3 ecNormal, float shininess, vec4 color)
{

    vec4 amb = vec4(0), diff = vec4(0), spec = vec4(0);

    PointLight(ecLight,
               vec3(0.0005, 0, 1),
               vec4(1),
               vec4(1),
               vec4(1),
               ecPosition,
               ecNormal,
               shininess,
               amb,
               diff,
               spec);

    amb = min(vec4(1), amb);
    diff = min(vec4(1), diff);
    spec = min(vec4(1), spec);

    vec4 shaded =
      amb * Ambient * color + diff * Diffuse * color + spec * Specular * color;

    return vec4(shaded.rgb, color.a);
}
