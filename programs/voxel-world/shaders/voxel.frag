

#include "directional-light.h"
#include "gamma.h"

uniform vec3 ecLightDir;
uniform vec4 materialProperties;

in vec3 ecPosition;
in vec3 ecNormal;
flat in vec4 vColor;
in float diffFactor;

out vec4 fragColor;

void
main()
{
    /* vec4 L = DirectionalLight(ecPosition, normalize(ecNormal), ecLightDir, */
    /*                           vec4(materialProperties.x),
     * vec4(materialProperties.z), */
    /*                           materialProperties.w); */

    /* fragColor = (L + vec4(materialProperties.y)) * vColor; */
    /* fragColor *= diff; */
    /* fragColor.a = 1; */

    float diff = min(diffFactor + 0.3, 1.0);

    fragColor = vColor;
    fragColor *= diff;
    fragColor.a = 1.0;
    fragColor = gammaCorrect(fragColor);
}
