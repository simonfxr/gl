#version 330

in vec3 vertex;
in vec3 normal;

uniform mat4 mvpMatrix;
uniform mat4 mvMatrix;
uniform mat3 normalMatrix;
uniform vec3 lightPosition;

const float SpecularContribution = 0.3;
const float DiffuseContribution = 1.0 - SpecularContribution;

out float lightIntensity;
out vec2  position;

void main()
{
    vec3 ecPosition = vec3(mvMatrix * vec4(vertex, 1));               
    vec3 tnorm      = normalize(normalMatrix * normal);      
    vec3 lightVec   = normalize(lightPosition - ecPosition);   
    vec3 reflectVec = reflect(-lightVec, tnorm);               
    vec3 viewVec    = normalize(-ecPosition);                  
    float diffuse   = max(dot(lightVec, tnorm), 0.0);          
    float spec      = 0.0;                                     

    if (diffuse > 0.0)
    {
        spec = max(dot(reflectVec, viewVec), 0.0);
        spec = pow(spec, 16.0);
    }

    lightIntensity = DiffuseContribution * diffuse + SpecularContribution * spec;
    
    position = vertex.xy;
    gl_Position = mvpMatrix * vec4(vertex, 1);
}
