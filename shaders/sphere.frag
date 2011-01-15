#version 330

uniform vec3 ecLight;
uniform vec4 color;
uniform float shininess;
uniform vec3 ecSpotDirection;
uniform float spotAngle;

const float Ambient = 0.65; // 0.7;
const float Diffuse = 0.2; // 0.2;
const float Specular = 0.15; // 0.1;

in vec3 ecPosition;
in vec3 ecNormal;

out vec4 fragColor;

void main() {
    
    vec3 light = normalize(ecLight - ecPosition);
    vec3 eyeNormal = normalize(ecNormal);
    float ambientIntensity = max(0, dot(eyeNormal, light));

    vec3 spectator = normalize(-ecPosition);
    vec3 lightReflect = reflect(-light, eyeNormal);
    float specularIntensity = pow(max(0, dot(spectator, lightReflect)), shininess);

    float diff = Diffuse;
    /* float ambient = Ambient; */
    /* float specular = Specular; */

    float spotEdge = 0.90;
    float spotFull = 0.91;

    float spotAngle = max(0, dot(ecSpotDirection, -light));
    float spotIntensity = smoothstep(spotEdge, spotFull, spotAngle);

    
    float ambient = mix(0.35, Ambient, spotIntensity);
    float specular = mix(0, Specular, spotIntensity);

/*     if (spotAngle < ) { */
/* //        diff = 0.6; */
/*         ambient = 0.4; */
/*         ambientIntensity *= 0.5; */
/*         specular = 0; */
/*     } */

    vec3 baseColor = vec3(color);
    vec3 tone = baseColor * diff +
                baseColor * ambient * ambientIntensity +
                vec3(0.8) * specular * specularIntensity;

    fragColor = vec4(tone, color.a);
}
