
// from the book "OpenGL Shading Language" with minor modificiations

void SpotLight(vec3 ecLight, vec3 attenuation,
               vec3 spotDirection, float spotCosCutoff, float spotExponent,
               vec3 lightAmbient, vec3 lightDiffuse, vec3 lightSpecular,
               vec3 ecPosition, vec3 ecNormal, vec3 shininess,
               inout vec4 ambient, inout vec4 diffuse, inout vec4 specular) {
    
    vec3 eye = normalize(-ecPosition);
    
    float nDotVP;
// normal . light direction
    float nDotHV;
// normal . light half vector
    float pf;
// power factor
    float spotDot;
// cosine of angle between spotlight
    float spotAttenuation; // spotlight attenuation factor
    float attenuation;
// computed attenuation factor
    float d;
// distance from surface to light source
    vec3 VP;
// direction from surface to light position
    vec3 halfVector;
// direction of maximum highlights
// Compute vector from surface to light position
    VP = vec3(LightSource[i].position) - ecPosition3;
// Compute distance between surface and light position
    d = length(VP);
    9.3 Material Properties and Lighting
// Normalize the vector from surface to light position
        VP = normalize(VP);
// Compute attenuation
    attenuationFactor = 1.0 / (attenuation.z +
                               attenuation.y * d +
                               attenuation.x * d * d);
// See if point on surface is inside cone of illumination
    spotDot = dot(-VP, normalize(spotDirection));
    if (spotDot < spotCosCutoff)
        spotAttenuation = 0.0; // light adds no contribution
    else
        spotAttenuation = pow(spotDot, spotExponent);
// Combine the spotlight and distance attenuation.
    attenuation *= spotAttenuation;
    halfVector = normalize(VP + eye);
    nDotVP = max(0.0, dot(normal, VP));
    nDotHV = max(0.0, dot(normal, halfVector));
    
    if (nDotVP == 0.0)
        pf = 0.0;
    else
        pf = pow(nDotHV, shininess);
    
    ambient += lightAmbient * attenuation;
    diffuse += lightDiffuse * nDotVP * attenuation;
    specular += lightSpecular * pf * attenuation;
}
