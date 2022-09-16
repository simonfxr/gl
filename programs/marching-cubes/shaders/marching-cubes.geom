
uniform usampler1D caseToNumPolysData;
uniform usampler1D triangleTableData;
uniform sampler3D worldVolume;
uniform mat4 worldMatrix;
uniform vec3 texEdgeDim;

int edgeTable(int cas) {
    return int(texelFetch(caseToNumPolysData, cas, 0).r);
}

int triangleTable(int cas, int num) {
    return int(texelFetch(triangleTableData, cas * 16 + num, 0).r);
}

float sampleVolume(vec3 uvw) {
    /* uvw = clamp(uvw, vec3(0), vec3(1)); */
    /* vec4 wc4 = worldMatrix * vec4(uvw, 1); */
    /* vec3 wc = wc4.xyz / wc4.w; */

    /* float rad = 2; */
    /* vec3 center = vec3(3, 1, 3); */


    /* vec3 diff = wc - center; */
    /* return dot(diff, diff) - rad * rad; */
    return texture(worldVolume, uvw).r;
}

layout(points) in;

layout(triangle_strip, max_vertices = 16) out;

out vec3 gPosition;
out vec3 gNormal;

vec3 interpolate(vec3 v1, float y1, vec3 v2, float y2) {
    return v1 + (v2 - v1) * (y1 / (y1 - y2));
}

vec3 gradientAt(vec3 uvw) {
    vec3 off = texEdgeDim;
    
    vec3 grad;
    grad.x = sampleVolume(uvw + vec3(off.x, 0, 0)) -
        sampleVolume(uvw - vec3(off.x, 0, 0));
    grad.y = sampleVolume(uvw + vec3(0, off.y, 0)) -
        sampleVolume(uvw - vec3(0, off.y, 0));
    grad.z = sampleVolume(uvw + vec3(0, 0, off.z)) -
        sampleVolume(uvw - vec3(0, 0, off.z));
    return grad;
}

void main() {
    vec3 tc0 = gl_in[0].gl_Position.xyz;
    vec3 tcstep = texEdgeDim;
        
    vec3 tcs[8];
    
    tcs[0] = tc0;
    tcs[1] = tc0 + vec3(tcstep[0], 0, 0);
    tcs[2] = tc0 + vec3(tcstep[0], 0, tcstep[2]);
    tcs[3] = tc0 + vec3(0, 0, tcstep[2]);
    tcs[4] = tc0 + vec3(0, tcstep[1], 0);
    tcs[5] = tc0 + vec3(tcstep[0], tcstep[1], 0);
    tcs[6] = tc0 + tcstep;
    tcs[7] = tc0 + vec3(0, tcstep[1], tcstep[2]);

    float vs[8];
    for (int i = 0; i < 8; ++i)
        vs[i] = sampleVolume(tcs[i]);

    int cubeIndex = 0;
    for (int i = 0; i < 8; ++i)
        cubeIndex |= int(vs[i] < 0) << i;

    int edgeCode = edgeTable(cubeIndex);
    if(edgeCode == 0)
        return;

    vec3 intVerts[12];

    if((edgeCode & 1) != 0) intVerts[0] = interpolate(tcs[0], vs[0], tcs[1], vs[1]);
    if((edgeCode & 2) != 0) intVerts[1] = interpolate(tcs[1], vs[1], tcs[2], vs[2]);
    if((edgeCode & 4) != 0) intVerts[2] = interpolate(tcs[2], vs[2], tcs[3], vs[3]);
    if((edgeCode & 8) != 0) intVerts[3] = interpolate(tcs[3], vs[3], tcs[0], vs[0]);
    if((edgeCode & 16) != 0) intVerts[4] = interpolate(tcs[4], vs[4], tcs[5], vs[5]);
    if((edgeCode & 32) != 0) intVerts[5] = interpolate(tcs[5], vs[5], tcs[6], vs[6]);
    if((edgeCode & 64) != 0) intVerts[6] = interpolate(tcs[6], vs[6], tcs[7], vs[7]);
    if((edgeCode & 128) != 0) intVerts[7] = interpolate(tcs[7], vs[7], tcs[4], vs[4]);
    if((edgeCode & 256) != 0) intVerts[8] = interpolate(tcs[0], vs[0], tcs[4], vs[4]);
    if((edgeCode & 512) != 0) intVerts[9] = interpolate(tcs[1], vs[1], tcs[5], vs[5]);
    if((edgeCode & 1024) != 0) intVerts[10] = interpolate(tcs[2], vs[2], tcs[6], vs[6]);
    if((edgeCode & 2048) != 0) intVerts[11] = interpolate(tcs[3], vs[3], tcs[7], vs[7]);

    for (int i = 0; triangleTable(cubeIndex, i) != 255; i += 3) {
        for (int j = 0; j < 3; ++j) {
            vec3 uvw = intVerts[triangleTable(cubeIndex, i + j)];
            gPosition = uvw;
            gNormal = normalize(gradientAt(uvw));
            EmitVertex();
        }

        EndPrimitive();
    }
}
