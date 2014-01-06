//#pragma OPENCL EXTENSION cl_amd_printf:enable
#pragma OPENCL EXTENSION cl_khr_3d_image_writes : enable

#define ISOLEVEL 32768

__constant sampler_t sampler = CLK_NORMALIZED_COORDS_FALSE | CLK_ADDRESS_CLAMP | CLK_FILTER_NEAREST;

__constant int4 cube_offsets[8] = {
    {0, 0, 0, 0},
    {1, 0, 0, 0},
    {0, 0, 1, 0},
    {1, 0, 1, 0},
    {0, 1, 0, 0},
    {1, 1, 0, 0},
    {0, 1, 1, 0},
    {1, 1, 1, 0},
};

__constant uchar triangle_count_table[256] = {
#include "cl/triangle_count_table.h"
};

__constant char offsets3[72] = {
    // 0
    0,0,0,
    1,0,0,
    // 1
    1,0,0,
    1,0,1,
    // 2
    1,0,1,
    0,0,1,
    // 3
    0,0,1,
    0,0,0,
    // 4
    0,1,0,
    1,1,0,
    // 5
    1,1,0,
    1,1,1,
    // 6
    1,1,1,
    0,1,1,
    // 7
    0,1,1,
    0,1,0,
    // 8
    0,0,0,
    0,1,0,
    // 9
    1,0,0,
    1,1,0,
    // 10
    1,0,1,
    1,1,1,
    // 11
    0,0,1,
    0,1,1
};

__constant char triangle_edge_table[4096] = {
#include "cl/triangle_edge_table.h"
};

__kernel void computeHistogramLevel(
    __read_only image3d_t source,
    __write_only image3d_t dest)
{
    int4 wpos = { get_global_id(0), get_global_id(1), get_global_id(2), 0 };
    int4 rpos = wpos * 2;

    uint count = 0;
    count += read_imageui(source, sampler, rpos + cube_offsets[0]).x;
    count += read_imageui(source, sampler, rpos + cube_offsets[1]).x;
    count += read_imageui(source, sampler, rpos + cube_offsets[2]).x;
    count += read_imageui(source, sampler, rpos + cube_offsets[3]).x;
    count += read_imageui(source, sampler, rpos + cube_offsets[4]).x;
    count += read_imageui(source, sampler, rpos + cube_offsets[5]).x;
    count += read_imageui(source, sampler, rpos + cube_offsets[6]).x;
    count += read_imageui(source, sampler, rpos + cube_offsets[7]).x;

    write_imageui(dest, wpos, count);
}

int4 scanHistogramLevel(int index, __read_only image3d_t histo, int4 pos) {

    int8 neighbors = {
        read_imagei(histo, sampler, pos + cube_offsets[0]).x,
        read_imagei(histo, sampler, pos + cube_offsets[1]).x,
        read_imagei(histo, sampler, pos + cube_offsets[2]).x,
        read_imagei(histo, sampler, pos + cube_offsets[3]).x,
        read_imagei(histo, sampler, pos + cube_offsets[4]).x,
        read_imagei(histo, sampler, pos + cube_offsets[5]).x,
        read_imagei(histo, sampler, pos + cube_offsets[6]).x,
        read_imagei(histo, sampler, pos + cube_offsets[7]).x
    };
    
    int acc = pos.s3;
    int8 cmp;

    acc += neighbors.s0;
    cmp.s0 = acc <= index;
    acc += neighbors.s1;
    cmp.s1 = acc <= index;
    acc += neighbors.s2;
    cmp.s2 = acc <= index;
    acc += neighbors.s3;
    cmp.s3 = acc <= index;
    acc += neighbors.s4;
    cmp.s4 = acc <= index;
    acc += neighbors.s5;
    cmp.s5 = acc <= index;
    acc += neighbors.s6;
    cmp.s6 = acc <= index;
    cmp.s7 = 0;

    pos += cube_offsets[cmp.s0 + cmp.s1 + cmp.s2 + cmp.s3 +
                        cmp.s4 + cmp.s5 + cmp.s6 + cmp.s7];

    pos.s0 *= 2;
    pos.s1 *= 2;
    pos.s2 *= 2;
    
    neighbors *= cmp;
    pos.s3 +=
        neighbors.s0 + neighbors.s1 + neighbors.s2 + neighbors.s3 +
        neighbors.s4 + neighbors.s5 + neighbors.s6 + neighbors.s7;
    
    return pos;
}

float sampleVolume(__read_only image3d_t volumeData, int3 p) {
    int x = (int) read_imageui(volumeData, sampler, (int4) (p.x, p.y, p.z, 0)).s0;
    return (float)(x - ISOLEVEL) * (1.f / (float) ISOLEVEL);
}

ushort make_iso(float x) {
    x = -x;
    x = clamp(x, -1.f, 1.f);
    x *= 32768;
    x += 32768;
    x = clamp(x, 0.f, 65535.f);
    return (ushort) (int) x;
}

void storeVolume(__write_only image3d_t vol, int4 p, float x) {
    write_imageui(vol, p, make_iso(x));
}

float3 calculateGradient(__read_only image3d_t volumeData, int3 p) {
    float dx = (float) sampleVolume(volumeData, p + (int3)(1, 0, 0)) - (float) sampleVolume(volumeData, p - (int3)(1, 0, 0));
    float dy = (float) sampleVolume(volumeData, p + (int3)(0, 1, 0)) - (float) sampleVolume(volumeData, p - (int3)(0, 1, 0));
    float dz = (float) sampleVolume(volumeData, p + (int3)(0, 0, 1)) - (float) sampleVolume(volumeData, p - (int3)(0, 0, 1));
    return (float3) (dx, dy, dz);
}

__kernel void histogramTraversal(
    __read_only image3d_t hp0, // Largest HP
    __read_only image3d_t hp1,
    __read_only image3d_t hp2,
    __read_only image3d_t hp3,
    __read_only image3d_t hp4,
    __read_only image3d_t hp5,
#if SIZE > 64
    __read_only image3d_t hp6,
#endif
#if SIZE > 128
    __read_only image3d_t hp7,
#endif
#if SIZE > 256
    __read_only image3d_t hp8, 
#endif
#if SIZE > 512
    __read_only image3d_t hp9, 
#endif
    __read_only image3d_t volume,
    __global float * VBOBuffer,
    __private int sum)
{
	
    int target = get_global_id(0);
    if(target >= sum)
        target = 0;
    
    int4 cubePosition = {0,0,0,0}; // x,y,z,sum
#if SIZE > 512
    cubePosition = scanHistogramLevel(target, hp9, cubePosition);
#endif
#if SIZE > 256
    cubePosition = scanHistogramLevel(target, hp8, cubePosition);
#endif
#if SIZE > 128
    cubePosition = scanHistogramLevel(target, hp7, cubePosition);
#endif
#if SIZE > 64
    cubePosition = scanHistogramLevel(target, hp6, cubePosition);
#endif
    cubePosition = scanHistogramLevel(target, hp5, cubePosition);
    cubePosition = scanHistogramLevel(target, hp4, cubePosition);
    cubePosition = scanHistogramLevel(target, hp3, cubePosition);
    cubePosition = scanHistogramLevel(target, hp2, cubePosition);
    cubePosition = scanHistogramLevel(target, hp1, cubePosition);
    cubePosition = scanHistogramLevel(target, hp0, cubePosition);
    cubePosition.x = cubePosition.x / 2;
    cubePosition.y = cubePosition.y / 2;
    cubePosition.z = cubePosition.z / 2;
    
    char vertexNr = 0;

    uint edge_code = read_imageui(hp0, sampler, cubePosition).s1;

    // max 5 triangles
    for(int i = (target-cubePosition.s3)*3; i < (target-cubePosition.s3+1)*3; i++) { // for each vertex in triangle
        const uchar edge = triangle_edge_table[edge_code*16 + i];
        const int3 point0 = (int3)(cubePosition.x + offsets3[edge*6], cubePosition.y + offsets3[edge*6+1], cubePosition.z + offsets3[edge*6+2]);
        const int3 point1 = (int3)(cubePosition.x + offsets3[edge*6+3], cubePosition.y + offsets3[edge*6+4], cubePosition.z + offsets3[edge*6+5]);
        
        // Store vertex in VBO
	
        const float3 forwardDifference0 = - calculateGradient(volume, point0);
        const float3 forwardDifference1 = - calculateGradient(volume, point1);
        
        float v0 = sampleVolume(volume, point0);
        float v1 = sampleVolume(volume, point1);
        const float diff = native_divide(v0, v0 - v1);
        
        const float3 vertex = mix((float3)(point0.x, point0.y, point0.z), (float3)(point1.x, point1.y, point1.z), diff);

        const float3 normal = normalize(mix(normalize(forwardDifference0), normalize(forwardDifference1), diff));
        
        
        vstore3(vertex, target*6 + vertexNr*2, VBOBuffer);
        vstore3(normal, target*6 + vertexNr*2 + 1, VBOBuffer);

        ++vertexNr;
    }
}




__kernel void computeHistogramBaseLevel(
    __write_only image3d_t histo_base,
    __read_only image3d_t data)
{
    ushort isolevel = ISOLEVEL;
    int4 pos = { get_global_id(0), get_global_id(1), get_global_id(2), 0 };

    uchar cube_index = 0;
    cube_index |= (read_imageui(data, sampler, pos + cube_offsets[0]).x > isolevel) << 0;
    cube_index |= (read_imageui(data, sampler, pos + cube_offsets[1]).x > isolevel) << 1;
    cube_index |= (read_imageui(data, sampler, pos + cube_offsets[3]).x > isolevel) << 2;
    cube_index |= (read_imageui(data, sampler, pos + cube_offsets[2]).x > isolevel) << 3;
    cube_index |= (read_imageui(data, sampler, pos + cube_offsets[4]).x > isolevel) << 4;
    cube_index |= (read_imageui(data, sampler, pos + cube_offsets[5]).x > isolevel) << 5;
    cube_index |= (read_imageui(data, sampler, pos + cube_offsets[7]).x > isolevel) << 6;
    cube_index |= (read_imageui(data, sampler, pos + cube_offsets[6]).x > isolevel) << 7;

    write_imageui(histo_base, pos, (uint4) { triangle_count_table[cube_index], cube_index, 0, 0});
}

typedef float16 mat4;
typedef float4 vec4;
typedef float4 vec3;

vec4 multM4V4(mat4 m, vec4 v) {
    float8 a = m.lo;
    float8 b = m.hi;
    
    float4 m1 = a.lo;
    float4 m2 = a.hi;
    float4 m3 = b.lo;
    float4 m4 = b.hi;

    vec4 c = (float4) 0;
    c += v.s0 * m1;
    c += v.s1 * m2;
    c += v.s2 * m3;
    c += v.s3 * m4;

    return c;
}

float dot3(vec3 a, vec3 b) {
    a *= b;
    return a.s0 + a.s1 + a.s2;
}

float4 qmul(float4 a, float4 b) {
    return (float4)
        (a.x * b.x - a.y * b.y - a.z * b.z - a.w * b.w,
         a.x * b.y + a.y * b.x + a.z * b.w - a.w * b.z,
         a.x * b.z - a.y * b.w + a.z * b.x + a.w * b.y,
         a.x * b.w + a.y * b.z - a.z * b.y + a.w * b.x);
}

float J(vec4 z, vec4 c) {
    for (int i = 0; i < 9; ++i)
        z = qmul(z, z) + c;
    return length(z);
}

__kernel void generateSphereVolume(__write_only image3d_t data,
                                   __private mat4 transform,
                                   __private vec3 center,
                                   __private float radius,
                                   __private float time)
{
    int4 pos = { get_global_id(0), get_global_id(1), get_global_id(2), 0 };
    /* float3 wc = convert_float3(pos.xyz); */
    /* wc -= (float3) (SIZE / 2); */
    /* wc /= (float)(SIZE / 2); */

    /* const float RAD = 1; */
    /* const float4 C = (float4)(-1, 0.2, sin(time * 0.1), 0); */
    
    /* float4 wc4 = (float4)(wc.xyz, 0); */
    /* float l = J(wc4, C) - 1; */
    /* float value = l; */
    
    
    vec4 wc = multM4V4(transform, convert_float4((int4)(pos.xyz, 1)));

    vec3 dist = wc - center;
    float value = dot3(dist, dist) / (radius * radius)  - 1;

    
    storeVolume(data, pos, value);
}
