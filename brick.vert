#version 330

const float SpecularContribution = 0.3;
const float DiffuseContribution = 1 - SpecularContribution;

in vec3 vertex;
in vec3 normal;

uniform mat4 mvpMatrix;
uniform mat4 mvMatrix;
uniform mat3 normalMatrix;
uniform vec3 lightPosition;

out float lightIntensity;
out vec2  vPosition;

void main()
{
    vec3 ecPosition = vec3(mvMatrix * vec4(vertex, 1));
    vec3 tnorm      = normalize(normalMatrix * normal);
    vec3 lightVec   = normalize(lightPosition - ecPosition);
    vec3 reflectVec = reflect(-lightVec, tnorm);
    vec3 viewVec    = normalize(-ecPosition);
    float diffuse   = max(dot(lightVec, tnorm), 0);
    float spec      = 0;

    if (diffuse > 0) {
        spec = max(dot(reflectVec, viewVec), 0);
        spec = pow(spec, 16);
    }

    lightIntensity = DiffuseContribution * diffuse +
                     SpecularContribution * spec;

    vPosition = vertex.xy;
    
    gl_Position = mvpMatrix * vec4(vertex, 1);
}
