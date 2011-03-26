#include "point_light.h"

uniform vec3 ecLight;
uniform vec4 materialProperties;

in vec3 ecPosition;
in vec3 ecNormal;
flat in vec4 vColor;

out vec4 fragColor;

void main() {
    vec4 amb, diff, spec;
    
    PointLight(ecLight, vec3(0, 0, 1),
               vec4(materialProperties.x), vec4(materialProperties.y),
               vec4(materialProperties.z), ecPosition, normalize(ecNormal),
               materialProperties.w,
               amb, diff, spec);
               
    fragColor = (amb + diff + spec) * vColor;
    fragColor.a = vColor.a;
}

