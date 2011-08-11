#ifndef GLT_PRIMITIVES_HPP
#define GLT_PRIMITIVES_HPP

#include "defs.h"
#include "error/error.hpp"

#include "math/vec3.hpp"
#include "math/vec2.hpp"
#include "math/ivec3.hpp"

#include "glt/CubeMesh.hpp"

namespace glt {

namespace primitives {

using namespace math;

template <typename V>
void rectangle(CubeMesh<V>& model, const math::point3_t& origin, const math::vec2_t& dim) {
    V vertex;
    vertex.position = origin; model.add(vertex);
    vertex.position = origin + vec3(dim[0], 0, 0); model.add(vertex);
    vertex.position = origin + vec3(dim[0], dim[1], 0); model.add(vertex);
    vertex.position = origin + vec3(0, dim[1], 0); model.add(vertex);
}

template <typename V>
void cubeoid(CubeMesh<V>& model, const math::point3_t& origin, const math::vec3_t& dim, bool evert = false) {

    // struct Face {
    //     vec3_t normal;
    //     uint16 s0;
    //     uint16 s1;
    // };

    // static const Face faces[] = {
    //     { vec3(1.f, 0.f, 0.f), 2, 4 }
    //     { vec3(-1.f, 0.f, 0.f), 3, 4 }
    //     { vec3(0.f, 1.f, 0.f), 4, 0 }
    //     { vec3(0.f, -1.f, 0.f), 4, 1 } 
    //     { vec3(0.f, 0.f, 1.f), 0, 2 }
    //     { vec3(0.f, 0.f, -1.f), 1, 2 }
    // };

    // const vec3_t dim2 = dim * 0.5f;
    
    // for (uint32 face = 0; face < 6; ++face) {
    //     vec3_t n = faces[face].normal;
    //     if (evert) n = -n;

    //     for (int j = -1; j < 2; j += 2) {
    //         for (int k = -1; k < 2; k += 2) {
    //             V vertex;
    //             vertex.normal = n;
    //             vertex.position = origin;
    //             vertex.position += dim2 * (evert ? -j : j) * faces[faces[face].s0].normal;
    //             vertex.position += dim2 * (evert ? -k : k) * faces[faces[face].s1].normal;
    //             model.addVertex(vertex);
    //         }
    //     }
    // }
    UNUSED(model);
    UNUSED(origin);
    UNUSED(dim);
    UNUSED(evert);
    ERR("not yet implemented");
    
}

template <typename Vertex>
void unitCubeWONormals(CubeMesh<Vertex>& cube) {
    Vertex v;

    v.position = vec3(0.f, 0.f,  1.0f); cube.add(v);
    v.position = vec3( 1.0f, 0.f,  1.0f); cube.add(v);
    v.position = vec3( 1.0f,  1.0f,  1.0f); cube.add(v);
    v.position = vec3(0.f,  1.0f,  1.0f); cube.add(v);

    v.position = vec3(0.f, 0.f, 0.f); cube.add(v);
    v.position = vec3(0.f,  1.0f, 0.f); cube.add(v);
    v.position = vec3( 1.0f,  1.0f, 0.f); cube.add(v);
    v.position = vec3( 1.0f, 0.f, 0.f); cube.add(v);

    v.position = vec3(0.f,  1.0f, 0.f); cube.add(v);
    v.position = vec3(0.f,  1.0f,  1.0f); cube.add(v);
    v.position = vec3( 1.0f,  1.0f,  1.0f); cube.add(v);
    v.position = vec3( 1.0f,  1.0f, 0.f); cube.add(v);

    v.position = vec3(0.f, 0.f, 0.f); cube.add(v);
    v.position = vec3( 1.0f, 0.f, 0.f); cube.add(v);
    v.position = vec3( 1.0f, 0.f,  1.0f); cube.add(v);
    v.position = vec3(0.f, 0.f,  1.0f); cube.add(v);

    v.position = vec3( 1.0f, 0.f, 0.f); cube.add(v);
    v.position = vec3( 1.0f,  1.0f, 0.f); cube.add(v);
    v.position = vec3( 1.0f,  1.0f,  1.0f); cube.add(v);
    v.position = vec3( 1.0f, 0.f,  1.0f); cube.add(v);

    v.position = vec3(0.f, 0.f, 0.f); cube.add(v);
    v.position = vec3(0.f, 0.f,  1.0f); cube.add(v);
    v.position = vec3(0.f,  1.0f,  1.0f); cube.add(v);
    v.position = vec3(0.f,  1.0f, 0.f); cube.add(v);
}

template <typename Vertex>
void unitCube(CubeMesh<Vertex>& cube) {
    Vertex v;

    v.normal = vec4(0.0f, 0.0f, 1.0f, 0.f);					
    v.position = vec3(0.f, 0.f,  1.0f); cube.add(v);
    v.position = vec3( 1.0f, 0.f,  1.0f); cube.add(v);
    v.position = vec3( 1.0f,  1.0f,  1.0f); cube.add(v);
    v.position = vec3(0.f,  1.0f,  1.0f); cube.add(v);

    v.normal = vec4( 0.0f, 0.0f, -1.f, 0.f);					
    v.position = vec3(0.f, 0.f, 0.f); cube.add(v);
    v.position = vec3(0.f,  1.0f, 0.f); cube.add(v);
    v.position = vec3( 1.0f,  1.0f, 0.f); cube.add(v);
    v.position = vec3( 1.0f, 0.f, 0.f); cube.add(v);

    v.normal = vec4( 0.0f, 1.0f, 0.0f, 0.f);					
    v.position = vec3(0.f,  1.0f, 0.f); cube.add(v);
    v.position = vec3(0.f,  1.0f,  1.0f); cube.add(v);
    v.position = vec3( 1.0f,  1.0f,  1.0f); cube.add(v);
    v.position = vec3( 1.0f,  1.0f, 0.f); cube.add(v);

    v.normal = vec4( 0.0f, -1.f, 0.0f, 0.f);					
    v.position = vec3(0.f, 0.f, 0.f); cube.add(v);
    v.position = vec3( 1.0f, 0.f, 0.f); cube.add(v);
    v.position = vec3( 1.0f, 0.f,  1.0f); cube.add(v);
    v.position = vec3(0.f, 0.f,  1.0f); cube.add(v);

    v.normal = vec4( 1.0f, 0.0f, 0.0f, 0.f);					
    v.position = vec3( 1.0f, 0.f, 0.f); cube.add(v);
    v.position = vec3( 1.0f,  1.0f, 0.f); cube.add(v);
    v.position = vec3( 1.0f,  1.0f,  1.0f); cube.add(v);
    v.position = vec3( 1.0f, 0.f,  1.0f); cube.add(v);

    v.normal = vec4(-1.f, 0.0f, 0.0f, 0.f);					
    v.position = vec3(0.f, 0.f, 0.f); cube.add(v);
    v.position = vec3(0.f, 0.f,  1.0f); cube.add(v);
    v.position = vec3(0.f,  1.0f,  1.0f); cube.add(v);
    v.position = vec3(0.f,  1.0f, 0.f); cube.add(v);
}

template <typename Vertex>
void unitCubeEverted(CubeMesh<Vertex>& cube) {
    Vertex v;
    
    v.normal = vec3(0.f, 0.f, -1.f);
    v.position = vec4(-1.0f, -1.0f,  1.0f, 1.f); cube.add(v);
    v.position = vec4( 1.0f, -1.0f,  1.0f, 1.f); cube.add(v);
    v.position = vec4( 1.0f,  1.0f,  1.0f, 1.f); cube.add(v);
    v.position = vec4(-1.0f,  1.0f,  1.0f, 1.f); cube.add(v);

    v.normal = vec3(0.f, 0.f, +1.f);
    v.position = vec4(-1.0f, -1.0f, -1.0f, 1.f); cube.add(v);
    v.position = vec4(-1.0f,  1.0f, -1.0f, 1.f); cube.add(v);
    v.position = vec4( 1.0f,  1.0f, -1.0f, 1.f); cube.add(v);
    v.position = vec4( 1.0f, -1.0f, -1.0f, 1.f); cube.add(v);

    v.normal = vec3(0.f, -1.f, 0.f);
    v.position = vec4(-1.0f,  1.0f, -1.0f, 1.f); cube.add(v);
    v.position = vec4(-1.0f,  1.0f,  1.0f, 1.f); cube.add(v);
    v.position = vec4( 1.0f,  1.0f,  1.0f, 1.f); cube.add(v);
    v.position = vec4( 1.0f,  1.0f, -1.0f, 1.f); cube.add(v);

    v.normal = vec3(0.f, +1.f, 0.f);
    v.position = vec4(-1.0f, -1.0f, -1.0f, 1.f); cube.add(v);
    v.position = vec4( 1.0f, -1.0f, -1.0f, 1.f); cube.add(v);
    v.position = vec4( 1.0f, -1.0f,  1.0f, 1.f); cube.add(v);
    v.position = vec4(-1.0f, -1.0f,  1.0f, 1.f); cube.add(v);

    v.normal = vec3(-1.f, 0.f, 0.f);
    v.position = vec4( 1.0f, -1.0f, -1.0f, 1.f); cube.add(v);
    v.position = vec4( 1.0f,  1.0f, -1.0f, 1.f); cube.add(v);
    v.position = vec4( 1.0f,  1.0f,  1.0f, 1.f); cube.add(v);
    v.position = vec4( 1.0f, -1.0f,  1.0f, 1.f); cube.add(v);

    v.normal = vec3(+1.f, 0.f, 0.f);
    v.position = vec4(-1.0f, -1.0f, -1.0f, 1.f); cube.add(v);
    v.position = vec4(-1.0f, -1.0f,  1.0f, 1.f); cube.add(v);
    v.position = vec4(-1.0f,  1.0f,  1.0f, 1.f); cube.add(v);
    v.position = vec4(-1.0f,  1.0f, -1.0f, 1.f); cube.add(v);
}

// from the GLTools library (OpenGL Superbible gltMakeSphere())
template <typename Vertex>
void sphere(Mesh<Vertex>& sphereBatch, GLfloat fRadius, GLint iSlices, GLint iStacks) {
    GLfloat drho = (GLfloat)(3.141592653589) / (GLfloat) iStacks;
    GLfloat dtheta = 2.0f * (GLfloat)(3.141592653589) / (GLfloat) iSlices;
    GLfloat ds = 1.0f / (GLfloat) iSlices;
    GLfloat dt = 1.0f / (GLfloat) iStacks;
    GLfloat t = 1.0f;	
    GLfloat s = 0.0f;
    GLint i, j;     // Looping variables
    
    for (i = 0; i < iStacks; i++) 
    {
        GLfloat rho = (GLfloat)i * drho;
        GLfloat srho = (GLfloat)(sin(rho));
        GLfloat crho = (GLfloat)(cos(rho));
        GLfloat srhodrho = (GLfloat)(sin(rho + drho));
        GLfloat crhodrho = (GLfloat)(cos(rho + drho));
		
        // Many sources of OpenGL sphere drawing code uses a triangle fan
        // for the caps of the sphere. This however introduces texturing 
        // artifacts at the poles on some OpenGL implementations
        s = 0.0f;
        vec3_t vVertex[4];
        vec3_t vNormal[4];
//        vec2_t vTexture[4];

        for ( j = 0; j < iSlices; j++) 
        {
            GLfloat theta = (j == iSlices) ? 0.0f : j * dtheta;
            GLfloat stheta = (GLfloat)(-sin(theta));
            GLfloat ctheta = (GLfloat)(cos(theta));
			
            GLfloat x = stheta * srho;
            GLfloat y = ctheta * srho;
            GLfloat z = crho;
        
            // vTexture[0][0] = s;
            // vTexture[0][1] = t;
            vNormal[0][0] = x;
            vNormal[0][1] = y;
            vNormal[0][2] = z;
            vVertex[0][0] = x * fRadius;
            vVertex[0][1] = y * fRadius;
            vVertex[0][2] = z * fRadius;
			
            x = stheta * srhodrho;
            y = ctheta * srhodrho;
            z = crhodrho;

            // vTexture[1][0] = s;
            // vTexture[1][1] = t - dt;
            vNormal[1][0] = x;
            vNormal[1][1] = y;
            vNormal[1][2] = z;
            vVertex[1][0] = x * fRadius;
            vVertex[1][1] = y * fRadius;
            vVertex[1][2] = z * fRadius;
			

            theta = ((j+1) == iSlices) ? 0.0f : (j+1) * dtheta;
            stheta = (GLfloat)(-sin(theta));
            ctheta = (GLfloat)(cos(theta));
			
            x = stheta * srho;
            y = ctheta * srho;
            z = crho;
        
            s += ds;
            // vTexture[2][0] = s;
            // vTexture[2][1] = t;
            vNormal[2][0] = x;
            vNormal[2][1] = y;
            vNormal[2][2] = z;
            vVertex[2][0] = x * fRadius;
            vVertex[2][1] = y * fRadius;
            vVertex[2][2] = z * fRadius;
			
            x = stheta * srhodrho;
            y = ctheta * srhodrho;
            z = crhodrho;

            // vTexture[3][0] = s;
            // vTexture[3][1] = t - dt;
            vNormal[3][0] = x;
            vNormal[3][1] = y;
            vNormal[3][2] = z;
            vVertex[3][0] = x * fRadius;
            vVertex[3][1] = y * fRadius;
            vVertex[3][2] = z * fRadius;

            for (uint32 i = 0; i < 3; ++i) {
                Vertex v;
                v.position = vec4(vVertex[i], 1.f);
                v.normal = vNormal[i];
                sphereBatch.addVertex(v);
            }
			
            // Rearrange for next triangle
            vVertex[0] = vVertex[1];
            vNormal[0] = vNormal[1];
            // vTexture[0] = vTexture[1];
			
            vVertex[1] = vVertex[3];
            vNormal[1] = vNormal[3];
            // vTexture[1] = vTexture[3];

            for (uint32 i = 0; i < 3; ++i) {
                Vertex v;
                v.position = vec4(vVertex[i], 1.f);
                v.normal = vNormal[i];
                sphereBatch.addVertex(v);
            }
        }
        t -= dt;
    }

    sphereBatch.drawType(DrawArrays);
    sphereBatch.primType(GL_TRIANGLES);
}

} // namespace primitives

} // namespace glt

#endif
