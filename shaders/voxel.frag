#include "directional-light.h"
#include "gamma.h"

uniform vec3 ecLightDir;
uniform vec4 materialProperties;

in vec3 ecPosition;
in vec3 ecNormal;
flat in vec4 vColor;

out vec4 fragColor;

void main() {
    vec4 L = DirectionalLight(ecPosition, normalize(ecNormal), ecLightDir,
                              vec4(materialProperties.x), vec4(materialProperties.z),
                              materialProperties.w);

    fragColor = (L + vec4(materialProperties.y)) * vColor;
    fragColor.a = vColor.a;
    fragColor = gammaCorrect(fragColor);
}

