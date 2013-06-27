

#include "point_light2.h"
#include "gamma.h"

const float AmbContr = 0.2;
const float DiffContr = 0.6;
const float SpecContr = 0.2;

uniform vec3 ecLight;

in vec3 ecPosition;
in vec3 ecNormal;
in vec3 color;
in float shininess;

out vec4 fragColor;

void main() {

    vec4 L = PointLight(ecPosition, normalize(ecNormal), ecLight,
                        vec4(DiffContr), vec4(SpecContr), shininess);

    fragColor = vec4((vec3(L) + vec3(AmbContr)) * color, 1.);
    fragColor = gammaCorrect(fragColor);
}