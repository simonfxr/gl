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
		__read_only image3d_t readHistoPyramid, 
		__write_only image3d_t writeHistoPyramid
	) {	

	int4 writePos = {get_global_id(0), get_global_id(1), get_global_id(2), 0};
	int4 readPos = writePos*2;
	int writeValue = read_imagei(readHistoPyramid, sampler, readPos).x + // 0
		read_imagei(readHistoPyramid, sampler, readPos+cube_offsets[1]).x + // 1
		read_imagei(readHistoPyramid, sampler, readPos+cube_offsets[2]).x + // 2
		read_imagei(readHistoPyramid, sampler, readPos+cube_offsets[3]).x + // 3
		read_imagei(readHistoPyramid, sampler, readPos+cube_offsets[4]).x + // 4
		read_imagei(readHistoPyramid, sampler, readPos+cube_offsets[5]).x + // 5
		read_imagei(readHistoPyramid, sampler, readPos+cube_offsets[6]).x + // 6
		read_imagei(readHistoPyramid, sampler, readPos+cube_offsets[7]).x; // 7

	write_imagei(writeHistoPyramid, writePos, writeValue);
}

int4 scanHPLevel(int target, __read_only image3d_t hp, int4 current) {
	
	int8 neighbors = {
		read_imagei(hp, sampler, current).x,
		read_imagei(hp, sampler, current + cube_offsets[1]).x,
		read_imagei(hp, sampler, current + cube_offsets[2]).x,
		read_imagei(hp, sampler, current + cube_offsets[3]).x,
		read_imagei(hp, sampler, current + cube_offsets[4]).x,
		read_imagei(hp, sampler, current + cube_offsets[5]).x,
		read_imagei(hp, sampler, current + cube_offsets[6]).x,
		read_imagei(hp, sampler, current + cube_offsets[7]).x
	};

	int acc = current.s3 + neighbors.s0;
	int8 cmp;
	cmp.s0 = acc <= target;
	acc += neighbors.s1;
	cmp.s1 = acc <= target;
	acc += neighbors.s2;
	cmp.s2 = acc <= target;
	acc += neighbors.s3;
	cmp.s3 = acc <= target;
	acc += neighbors.s4;
	cmp.s4 = acc <= target;
	acc += neighbors.s5;
	cmp.s5 = acc <= target;
	acc += neighbors.s6;
	cmp.s6 = acc <= target;
	cmp.s7 = 0;


	current += cube_offsets[(cmp.s0+cmp.s1+cmp.s2+cmp.s3+cmp.s4+cmp.s5+cmp.s6+cmp.s7)];
	current.s0 = current.s0*2;
	current.s1 = current.s1*2;
	current.s2 = current.s2*2;
	current.s3 = current.s3 +
		cmp.s0*neighbors.s0 + 
		cmp.s1*neighbors.s1 + 
		cmp.s2*neighbors.s2 + 
		cmp.s3*neighbors.s3 + 
		cmp.s4*neighbors.s4 + 
		cmp.s5*neighbors.s5 + 
		cmp.s6*neighbors.s6 + 
		cmp.s7*neighbors.s7;
	return current;

}

float sampleVolume(__read_only image3d_t volumeData, int3 p) {
    return (int) read_imageui(volumeData, sampler, (int4) (p.x, p.y, p.z, 0)).s0;
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
    cubePosition = scanHPLevel(target, hp9, cubePosition);
    #endif
    #if SIZE > 256
    cubePosition = scanHPLevel(target, hp8, cubePosition);
    #endif
    #if SIZE > 128
    cubePosition = scanHPLevel(target, hp7, cubePosition);
    #endif
    #if SIZE > 64
    cubePosition = scanHPLevel(target, hp6, cubePosition);
    #endif
    cubePosition = scanHPLevel(target, hp5, cubePosition);
    cubePosition = scanHPLevel(target, hp4, cubePosition);
    cubePosition = scanHPLevel(target, hp3, cubePosition);
    cubePosition = scanHPLevel(target, hp2, cubePosition);
    cubePosition = scanHPLevel(target, hp1, cubePosition);
    cubePosition = scanHPLevel(target, hp0, cubePosition);
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
        
        int v0 = sampleVolume(volume, point0);
        int v1 = sampleVolume(volume, point1);
        const float diff = native_divide((float)(ISOLEVEL-v0), (float)(v1 - v0));
        
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

/* __kernel void computeHistogramBaseLevel( */
/* 		__write_only image3d_t histoPyramid,  */
/* 		__read_only image3d_t rawData, */
/* 		__private int isolevel */
/* 		) { */
/*     int4 pos = {get_global_id(0), get_global_id(1), get_global_id(2), 0}; */

/*     // Find cube class nr */
/* 	const uchar first = read_imagei(rawData, sampler, pos).x; */
/*     const uchar cubeindex =  */
/*     ((first > isolevel)) | */
/*     ((read_imagei(rawData, sampler, pos + cube_offsets[1]).x > isolevel) << 1) | */
/*     ((read_imagei(rawData, sampler, pos + cube_offsets[3]).x > isolevel) << 2) | */
/*     ((read_imagei(rawData, sampler, pos + cube_offsets[2]).x > isolevel) << 3) | */
/*     ((read_imagei(rawData, sampler, pos + cube_offsets[4]).x > isolevel) << 4) | */
/*     ((read_imagei(rawData, sampler, pos + cube_offsets[5]).x > isolevel) << 5) | */
/*     ((read_imagei(rawData, sampler, pos + cube_offsets[7]).x > isolevel) << 6) | */
/*     ((read_imagei(rawData, sampler, pos + cube_offsets[6]).x > isolevel) << 7); */

/*     // Store number of triangles */
/* 	write_imageui(histoPyramid, pos, (uint4)(triangle_count_table[cubeindex], cubeindex, first, 0)); */
/* } */

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

ushort make_iso(float x) {
    x = clamp(x, -1.f, 1.f);
    x *= 32768;
    x += 32768;
    x = clamp(x, 0.f, 65535.f);
    x = 65535.f - x;
    return (ushort) (int) x;
}

__kernel void generateSphereVolume(__write_only image3d_t data,
                                   __private mat4 transform,
                                   __private vec3 center,
                                   __private float radius)
{
    int4 pos = { get_global_id(0), get_global_id(1), get_global_id(2), 1 };
    vec4 wc = multM4V4(transform, convert_float4(pos));

    vec3 dist = wc - center;
    float value = dot3(dist, dist) / (radius * radius)  - 1;

    write_imageui(data, pos, make_iso(value));
}
