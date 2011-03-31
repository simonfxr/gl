#include "directional-light.h"
#include "gamma.h"

uniform vec3 ecLightDir;
uniform vec4 materialProperties;

in vec3 ecPosition;
in vec3 ecNormal;
flat in vec4 vColor;
in float diffFactor;

out vec4 fragColor;

void main() {
    /* vec4 L = DirectionalLight(ecPosition, normalize(ecNormal), ecLightDir, */
    /*                           vec4(materialProperties.x), vec4(materialProperties.z), */
    /*                           materialProperties.w); */

    /* const vec4 lightColor = vec4(1); */

    /* float phi = asin(sin_time); */
    /* vec3 filter = abs(vec3(sin(0.3 * (phi + 3)), sin(0.6 * phi), sin(0.3 * phi))); */
    
    /* fragColor = (L + vec4(materialProperties.y)) * vec4(filter, 1) * lightColor * vColor; */
    /* fragColor *= diffFactor; */
    /* fragColor.a = 1; */

    fragColor = vec4(diffFactor, 0, 0, 1);
    fragColor = gammaCorrect(fragColor);
}

