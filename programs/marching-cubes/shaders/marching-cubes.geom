
uniform usampler1D caseToNumPolysData;
uniform usampler1D triangleTableData;
uniform sampler3D worldVolume;
uniform vec3 edgeDim;
uniform vec3 texEdgeDim;
uniform float isoLevel;
uniform mat4 vpMatrix;

const uint MAX_USHORT = 65535;

uint edgeTable(int cas) {
    return texelFetch(caseToNumPolysData, cas, 0).r;
}

uint triangleTable(int cas, int num) {
    return texelFetch(triangleTableData, cas * 16 + num, 0).r;
}

float noise3D(vec3 wc) {
    return texture(worldVolume, wc).r;
}

vec3 noise3D3(vec3 p) {
    return vec3(noise3D(p + vec3(0.089f, -0.193f, 0.521f)),
                noise3D(p + vec3(-0.239f, -0.181f, 0.619f)),
                noise3D(p + vec3(-0.107f, 0.157f, 0.487f)));
}

float sumNoise3D(vec3 p, uint octaves, float fmax, float fuzz, float damp) {
    float a = 1.f;
    float f = fmax;
    float y = 0.f;

    float sign = 1.f;
    
    for (uint i = 0; i < octaves; ++i) {
        y += noise3D(p * f) * a;
        f = (f / 2) + sign * fuzz;
        fuzz *= damp;
        a *= 2;
        sign = - sign;
    }

    return y;
}

float sampleVolume(vec3 p) {
    vec3 ws_orig = p;
    vec3 ws = p;
    float density = 0.f;

    float warp_freq = 0.006f;
    float warp_stride = 20.f;
    ws += noise3D3(ws * warp_freq) * warp_stride;

    density += -ws.y;

    float a0 = 0.6f;
    float f0 = 1.8f;

    /* density = noise3D(p * 0.05f); */

    /* density += noise3D(ws * f0 * 4.03) * 0.25 * a0; */
    /* density += noise3D(ws * f0 * 1.96) * 0.50 * a0; */
    /* density += noise3D(ws * f0 * 1.01) * 1.00 * a0; */

//    density += sumNoise3D(ws, 6, f0, 0.03f, 0.5192f) * a0;
    return density;
}

/* float sampleVolume(vec3 coord) { */
/*     coord = coord * 2 - vec3(1); */
/*     return dot(coord, coord) - 1; */
/* } */

layout(points) in;
layout(triangle_strip, max_vertices = 15) out;

in vec3 vTexCoord[1];

vec3 interpolate(vec3 v1, float y1, vec3 v2, float y2) {
    return v1 + (v2 - v1) * (y1 / (y1 - y2));
}

void main() {
    vec3 wc0 = gl_in[0].gl_Position.xyz;
    vec3 tc0 = vTexCoord[0];
    vec3 wcstep = edgeDim;
    vec3 tcstep = texEdgeDim;
        
    vec3 wcs[8];
    
    wcs[0] = wc0;
    wcs[1] = wc0 + vec3(wcstep[0], 0, 0);
    wcs[2] = wc0 + vec3(wcstep[0], 0, wcstep[2]);
    wcs[3] = wc0 + vec3(0, 0, wcstep[2]);
    wcs[4] = wc0 + vec3(0, wcstep[1], 0);
    wcs[5] = wc0 + vec3(wcstep[0], wcstep[1], 0);
    wcs[6] = wc0 + wcstep;
    wcs[7] = wc0 + vec3(0, wcstep[1], wcstep[2]);

    float vs[8];
    /* vs[0] = sampleVolume(tc0); */
    /* vs[1] = sampleVolume(tc0 + vec3(tcstep[0], 0, 0)); */
    /* vs[2] = sampleVolume(tc0 + vec3(tcstep[0], 0, tcstep[2])); */
    /* vs[3] = sampleVolume(tc0 + vec3(0, 0, tcstep[2])); */
    /* vs[4] = sampleVolume(tc0 + vec3(0, tcstep[1], 0)); */
    /* vs[5] = sampleVolume(tc0 + vec3(tcstep[0], tcstep[1], 0)); */
    /* vs[6] = sampleVolume(tc0 + tcstep); */
    /* vs[7] = sampleVolume(tc0 + vec3(0, tcstep[1], tcstep[2])); */

    for (int i = 0; i < 8; ++i)
        vs[i] = sampleVolume(wcs[i]);

    int cubeIndex = 0;
    for (int i = 0; i < 8; ++i)
        cubeIndex |= int(vs[i] < 0) << i;

    uint edgeCode = edgeTable(cubeIndex);
    if(edgeCode == 0)
        return;

    vec3 intVerts[12];

    if((edgeCode & 1) != 0) intVerts[0] = interpolate(wcs[0], vs[0], wcs[1], vs[1]);
    if((edgeCode & 2) != 0) intVerts[1] = interpolate(wcs[1], vs[1], wcs[2], vs[2]);
    if((edgeCode & 4) != 0) intVerts[2] = interpolate(wcs[2], vs[2], wcs[3], vs[3]);
    if((edgeCode & 8) != 0) intVerts[3] = interpolate(wcs[3], vs[3], wcs[0], vs[0]);
    if((edgeCode & 16) != 0) intVerts[4] = interpolate(wcs[4], vs[4], wcs[5], vs[5]);
    if((edgeCode & 32) != 0) intVerts[5] = interpolate(wcs[5], vs[5], wcs[6], vs[6]);
    if((edgeCode & 64) != 0) intVerts[6] = interpolate(wcs[6], vs[6], wcs[7], vs[7]);
    if((edgeCode & 128) != 0) intVerts[7] = interpolate(wcs[7], vs[7], wcs[4], vs[4]);
    if((edgeCode & 256) != 0) intVerts[8] = interpolate(wcs[0], vs[0], wcs[4], vs[4]);
    if((edgeCode & 512) != 0) intVerts[9] = interpolate(wcs[1], vs[1], wcs[5], vs[5]);
    if((edgeCode & 1024) != 0) intVerts[10] = interpolate(wcs[2], vs[2], wcs[6], vs[6]);
    if((edgeCode & 2048) != 0) intVerts[11] = interpolate(wcs[3], vs[3], wcs[7], vs[7]);

    for (int i = 0; triangleTable(cubeIndex, i) != 255; i += 3) {
        for (int j = 0; j < 3; ++j) {
            gl_Position = vpMatrix * vec4(intVerts[triangleTable(cubeIndex, i + j)], 1);
            EmitVertex();
        }

        EndPrimitive();
    }
}
