
// from the book "OpenGL Shading Language" with minor modificiations

void PointLight(vec3 ecLight, vec3 attenuation,
                vec3 lightAmbient, vec3 lightDiffuse, vec3 lightSpecular,
                vec3 ecPosition, vec3 ecNormal, float shininess,
                inout vec4 ambient, inout vec4 diffuse, inout vec4 specular) {


    vec3 eye = normalize(-ecPosition);
    
    float nDotVP;
// normal . light direction
    float nDotHV;
// normal . light half vector
    float pf;
// power factor
    float attenuationFactor; // computed attenuation factor
    float d;
// distance from surface to light source
    vec3 VP;
// direction from surface to light position
    vec3 halfVector;
// direction of maximum highlights
// Compute vector from surface to light position
    VP = ecLight - ecPosition;
// Compute distance between surface and light position
    d = length(VP);
// Normalize the vector from surface to light position
    VP = normalize(VP);
// Compute attenuation
    
    attenuationFactor = 1.0 / (attenuation.z +
                               attenuation.y * d +
                               attenuation.x * d * d);
    
    halfVector = normalize(VP + eye);
    nDotVP = max(0.0, dot(ecNormal, VP));
    nDotHV = max(0.0, dot(ecNormal, halfVector));
    if (nDotVP == 0.0)
        pf = 0.0;
    else
        pf = pow(nDotHV, shininess);
    
    ambient += lightAmbient * attenuationFactor;
    diffuse += lightDiffuse * nDotVP * attenuationFactor;
    specular += lightSpecular * pf * attenuationFactor;
}
