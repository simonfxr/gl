#version 330

const vec3 BrickColor  = vec3(0.8, 0, 0);
const vec3 MortarColor = vec3(0.6);
const vec2 BrickSize   = vec2(0.06, 0.03);
const vec2 BrickPct    = vec2(0.9, 0.85);

const float SpecularContribution = 0.2;
const float DiffuseContribution = 1 - SpecularContribution;

uniform mat4 mvMatrix;
uniform mat3 normalMatrix;
uniform vec3 lightPosition;

in vec3 mcVertex;
in vec3 mcNormal;

/* in float lightIntensity; */
/* in vec2  vPosition; */

out vec4 fragColor;

void main()
{

    vec3 ecPosition = vec3(mvMatrix * vec4(mcVertex, 1));
    vec3 tnorm      = normalize(normalMatrix * mcNormal);
    vec3 lightVec   = normalize(lightPosition - ecPosition);
    vec3 reflectVec = reflect(-lightVec, tnorm);
    vec3 viewVec    = normalize(-ecPosition);
    float diffuse   = max(dot(lightVec, tnorm), 0);
    float spec      = 0;

    if (diffuse > 0) {
        spec = max(dot(reflectVec, viewVec), 0);
        spec = pow(spec, 4);
    }

    float lightIntensity = DiffuseContribution * diffuse +
                           SpecularContribution * spec;

    vec2 vPosition;
    if (abs(mcNormal.z) != 0)
        vPosition = mcVertex.xy;
    else if (abs(mcNormal.y) != 0)
        vPosition = mcVertex.xz;
    else
        vPosition = mcVertex.zy;

    
    vec3 color;
    vec2 position, useBrick;

    position = vPosition / BrickSize;

    if (fract(position.y * 0.5) > 0.5)
        position.x += 0.5;

    position = fract(position);

    useBrick = step(position, BrickPct);

    color = mix(MortarColor, BrickColor, useBrick.x * useBrick.y);
    color *= lightIntensity;
    fragColor = vec4(color, 1);
}
