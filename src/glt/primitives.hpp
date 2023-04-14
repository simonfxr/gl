#ifndef GLT_PRIMITIVES_HPP
#define GLT_PRIMITIVES_HPP

#include "defs.h"
#include "err/err.hpp"

#include "math/fvec.hpp"
#include "math/ivec.hpp"

#include "glt/CubeMesh.hpp"

namespace glt {

namespace primitives {

constexpr void
setPoint(math::vec3 &a, const math::vec3 &b)
{
    a = b;
}

constexpr void
setPoint(math::vec4 &a, const math::vec3 &b)
{
    a = math::vec4(b, math::real(1));
}

constexpr void
setVec(math::vec3 &a, const math::vec3 &b)
{
    a = b;
}

constexpr void
setVec(math::vec4 &a, const math::vec3 &b)
{
    a = math::vec4(b, math::real(0));
}

template<typename V>
void
rectangle(CubeMesh<V> &model, const math::point3 &origin, const math::vec2 &dim)
{
    using namespace math;
    V vertex;
    vertex.position = origin;
    model.add(vertex);
    vertex.position = origin + vec3(dim[0], 0, 0);
    model.add(vertex);
    vertex.position = origin + vec3(dim[0], dim[1], 0);
    model.add(vertex);
    vertex.position = origin + vec3(0, dim[1], 0);
    model.add(vertex);
}

template<typename V>
void
cubeoid(CubeMesh<V> &model,
        const math::point3 &origin,
        const math::vec3 &dim,
        bool evert = false)
{

    // struct Face {
    //     vec3 normal;
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

    // const vec3 dim2 = dim * 0.5f;

    // for (uint32_t face = 0; face < 6; ++face) {
    //     vec3 n = faces[face].normal;
    //     if (evert) n = -n;

    //     for (int j = -1; j < 2; j += 2) {
    //         for (int k = -1; k < 2; k += 2) {
    //             V vertex;
    //             vertex.normal = n;
    //             vertex.position = origin;
    //             vertex.position += dim2 * (evert ? -j : j) *
    //             faces[faces[face].s0].normal; vertex.position += dim2 *
    //             (evert ? -k : k) * faces[faces[face].s1].normal;
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

template<typename Vertex>
void
unitCubeWONormals(CubeMesh<Vertex> &cube)
{
    using namespace math;
    Vertex v;

    v.position = vec3(0.f, 0.f, 1.0f);
    cube.add(v);
    v.position = vec3(1.0f, 0.f, 1.0f);
    cube.add(v);
    v.position = vec3(1.0f, 1.0f, 1.0f);
    cube.add(v);
    v.position = vec3(0.f, 1.0f, 1.0f);
    cube.add(v);

    v.position = vec3(0.f, 0.f, 0.f);
    cube.add(v);
    v.position = vec3(0.f, 1.0f, 0.f);
    cube.add(v);
    v.position = vec3(1.0f, 1.0f, 0.f);
    cube.add(v);
    v.position = vec3(1.0f, 0.f, 0.f);
    cube.add(v);

    v.position = vec3(0.f, 1.0f, 0.f);
    cube.add(v);
    v.position = vec3(0.f, 1.0f, 1.0f);
    cube.add(v);
    v.position = vec3(1.0f, 1.0f, 1.0f);
    cube.add(v);
    v.position = vec3(1.0f, 1.0f, 0.f);
    cube.add(v);

    v.position = vec3(0.f, 0.f, 0.f);
    cube.add(v);
    v.position = vec3(1.0f, 0.f, 0.f);
    cube.add(v);
    v.position = vec3(1.0f, 0.f, 1.0f);
    cube.add(v);
    v.position = vec3(0.f, 0.f, 1.0f);
    cube.add(v);

    v.position = vec3(1.0f, 0.f, 0.f);
    cube.add(v);
    v.position = vec3(1.0f, 1.0f, 0.f);
    cube.add(v);
    v.position = vec3(1.0f, 1.0f, 1.0f);
    cube.add(v);
    v.position = vec3(1.0f, 0.f, 1.0f);
    cube.add(v);

    v.position = vec3(0.f, 0.f, 0.f);
    cube.add(v);
    v.position = vec3(0.f, 0.f, 1.0f);
    cube.add(v);
    v.position = vec3(0.f, 1.0f, 1.0f);
    cube.add(v);
    v.position = vec3(0.f, 1.0f, 0.f);
    cube.add(v);
}

template<typename Vertex>
void
unitCube(CubeMesh<Vertex> &cube)
{
    using namespace math;
    Vertex v;

    setVec(v.normal, vec3(0.0f, 0.0f, 1.0f));
    setPoint(v.position, vec3(0.f, 0.f, 1.0f));
    cube.add(v);
    setPoint(v.position, vec3(1.0f, 0.f, 1.0f));
    cube.add(v);
    setPoint(v.position, vec3(1.0f, 1.0f, 1.0f));
    cube.add(v);
    setPoint(v.position, vec3(0.f, 1.0f, 1.0f));
    cube.add(v);

    setVec(v.normal, vec3(0.0f, 0.0f, -1.f));
    setPoint(v.position, vec3(0.f, 0.f, 0.f));
    cube.add(v);
    setPoint(v.position, vec3(0.f, 1.0f, 0.f));
    cube.add(v);
    setPoint(v.position, vec3(1.0f, 1.0f, 0.f));
    cube.add(v);
    setPoint(v.position, vec3(1.0f, 0.f, 0.f));
    cube.add(v);

    setVec(v.normal, vec3(0.0f, 1.0f, 0.0f));
    setPoint(v.position, vec3(0.f, 1.0f, 0.f));
    cube.add(v);
    setPoint(v.position, vec3(0.f, 1.0f, 1.0f));
    cube.add(v);
    setPoint(v.position, vec3(1.0f, 1.0f, 1.0f));
    cube.add(v);
    setPoint(v.position, vec3(1.0f, 1.0f, 0.f));
    cube.add(v);

    setVec(v.normal, vec3(0.0f, -1.f, 0.0f));
    setPoint(v.position, vec3(0.f, 0.f, 0.f));
    cube.add(v);
    setPoint(v.position, vec3(1.0f, 0.f, 0.f));
    cube.add(v);
    setPoint(v.position, vec3(1.0f, 0.f, 1.0f));
    cube.add(v);
    setPoint(v.position, vec3(0.f, 0.f, 1.0f));
    cube.add(v);

    setVec(v.normal, vec3(1.0f, 0.0f, 0.0f));
    setPoint(v.position, vec3(1.0f, 0.f, 0.f));
    cube.add(v);
    setPoint(v.position, vec3(1.0f, 1.0f, 0.f));
    cube.add(v);
    setPoint(v.position, vec3(1.0f, 1.0f, 1.0f));
    cube.add(v);
    setPoint(v.position, vec3(1.0f, 0.f, 1.0f));
    cube.add(v);

    setVec(v.normal, vec3(-1.f, 0.0f, 0.0f));
    setPoint(v.position, vec3(0.f, 0.f, 0.f));
    cube.add(v);
    setPoint(v.position, vec3(0.f, 0.f, 1.0f));
    cube.add(v);
    setPoint(v.position, vec3(0.f, 1.0f, 1.0f));
    cube.add(v);
    setPoint(v.position, vec3(0.f, 1.0f, 0.f));
    cube.add(v);
}

template<typename Vertex>
void
unitCube3(CubeMesh<Vertex> &cube)
{
    using namespace math;
    Vertex v;

    setVec(v.normal, vec3(0.0f, 0.0f, 1.0f));
    setPoint(v.position, vec3(0.f, 0.f, 1.0f));
    v.texCoord = vec2(0, 0);
    cube.add(v);
    setPoint(v.position, vec3(1.0f, 0.f, 1.0f));
    v.texCoord = vec2(0, 1);
    cube.add(v);
    setPoint(v.position, vec3(1.0f, 1.0f, 1.0f));
    v.texCoord = vec2(1, 1);
    cube.add(v);
    setPoint(v.position, vec3(0.f, 1.0f, 1.0f));
    v.texCoord = vec2(1, 0);
    cube.add(v);

    setVec(v.normal, vec3(0.0f, 0.0f, -1.f));
    setPoint(v.position, vec3(0.f, 0.f, 0.f));
    v.texCoord = vec2(0, 0);
    cube.add(v);
    setPoint(v.position, vec3(0.f, 1.0f, 0.f));
    v.texCoord = vec2(0, 1);
    cube.add(v);
    setPoint(v.position, vec3(1.0f, 1.0f, 0.f));
    v.texCoord = vec2(1, 1);
    cube.add(v);
    setPoint(v.position, vec3(1.0f, 0.f, 0.f));
    v.texCoord = vec2(1, 0);
    cube.add(v);

    setVec(v.normal, vec3(0.0f, 1.0f, 0.0f));
    setPoint(v.position, vec3(0.f, 1.0f, 0.f));
    v.texCoord = vec2(0, 0);
    cube.add(v);
    setPoint(v.position, vec3(0.f, 1.0f, 1.0f));
    v.texCoord = vec2(0, 1);
    cube.add(v);
    setPoint(v.position, vec3(1.0f, 1.0f, 1.0f));
    v.texCoord = vec2(1, 1);
    cube.add(v);
    setPoint(v.position, vec3(1.0f, 1.0f, 0.f));
    v.texCoord = vec2(1, 0);
    cube.add(v);

    setVec(v.normal, vec3(0.0f, -1.f, 0.0f));
    setPoint(v.position, vec3(0.f, 0.f, 0.f));
    v.texCoord = vec2(0, 0);
    cube.add(v);
    setPoint(v.position, vec3(1.0f, 0.f, 0.f));
    v.texCoord = vec2(1, 0);
    cube.add(v);
    setPoint(v.position, vec3(1.0f, 0.f, 1.0f));
    v.texCoord = vec2(1, 1);
    cube.add(v);
    setPoint(v.position, vec3(0.f, 0.f, 1.0f));
    v.texCoord = vec2(0, 1);
    cube.add(v);

    setVec(v.normal, vec3(1.0f, 0.0f, 0.0f));
    setPoint(v.position, vec3(1.0f, 0.f, 0.f));
    v.texCoord = vec2(0, 0);
    cube.add(v);
    setPoint(v.position, vec3(1.0f, 1.0f, 0.f));
    v.texCoord = vec2(1, 0);
    cube.add(v);
    setPoint(v.position, vec3(1.0f, 1.0f, 1.0f));
    v.texCoord = vec2(1, 1);
    cube.add(v);
    setPoint(v.position, vec3(1.0f, 0.f, 1.0f));
    v.texCoord = vec2(0, 1);
    cube.add(v);

    setVec(v.normal, vec3(-1.f, 0.0f, 0.0f));
    setPoint(v.position, vec3(0.f, 0.f, 0.f));
    v.texCoord = vec2(0, 0);
    cube.add(v);
    setPoint(v.position, vec3(0.f, 0.f, 1.0f));
    v.texCoord = vec2(0, 1);
    cube.add(v);
    setPoint(v.position, vec3(0.f, 1.0f, 1.0f));
    v.texCoord = vec2(1, 1);
    cube.add(v);
    setPoint(v.position, vec3(0.f, 1.0f, 0.f));
    v.texCoord = vec2(1, 0);
    cube.add(v);
}

template<typename Vertex>
void
unitCubeEverted(CubeMesh<Vertex> &cube)
{
    using namespace math;
    Vertex v;

    v.normal = vec3(0.f, 0.f, -1.f);
    v.position = vec4(-1.0f, -1.0f, 1.0f, 1.f);
    cube.add(v);
    v.position = vec4(1.0f, -1.0f, 1.0f, 1.f);
    cube.add(v);
    v.position = vec4(1.0f, 1.0f, 1.0f, 1.f);
    cube.add(v);
    v.position = vec4(-1.0f, 1.0f, 1.0f, 1.f);
    cube.add(v);

    v.normal = vec3(0.f, 0.f, +1.f);
    v.position = vec4(-1.0f, -1.0f, -1.0f, 1.f);
    cube.add(v);
    v.position = vec4(-1.0f, 1.0f, -1.0f, 1.f);
    cube.add(v);
    v.position = vec4(1.0f, 1.0f, -1.0f, 1.f);
    cube.add(v);
    v.position = vec4(1.0f, -1.0f, -1.0f, 1.f);
    cube.add(v);

    v.normal = vec3(0.f, -1.f, 0.f);
    v.position = vec4(-1.0f, 1.0f, -1.0f, 1.f);
    cube.add(v);
    v.position = vec4(-1.0f, 1.0f, 1.0f, 1.f);
    cube.add(v);
    v.position = vec4(1.0f, 1.0f, 1.0f, 1.f);
    cube.add(v);
    v.position = vec4(1.0f, 1.0f, -1.0f, 1.f);
    cube.add(v);

    v.normal = vec3(0.f, +1.f, 0.f);
    v.position = vec4(-1.0f, -1.0f, -1.0f, 1.f);
    cube.add(v);
    v.position = vec4(1.0f, -1.0f, -1.0f, 1.f);
    cube.add(v);
    v.position = vec4(1.0f, -1.0f, 1.0f, 1.f);
    cube.add(v);
    v.position = vec4(-1.0f, -1.0f, 1.0f, 1.f);
    cube.add(v);

    v.normal = vec3(-1.f, 0.f, 0.f);
    v.position = vec4(1.0f, -1.0f, -1.0f, 1.f);
    cube.add(v);
    v.position = vec4(1.0f, 1.0f, -1.0f, 1.f);
    cube.add(v);
    v.position = vec4(1.0f, 1.0f, 1.0f, 1.f);
    cube.add(v);
    v.position = vec4(1.0f, -1.0f, 1.0f, 1.f);
    cube.add(v);

    v.normal = vec3(+1.f, 0.f, 0.f);
    v.position = vec4(-1.0f, -1.0f, -1.0f, 1.f);
    cube.add(v);
    v.position = vec4(-1.0f, -1.0f, 1.0f, 1.f);
    cube.add(v);
    v.position = vec4(-1.0f, 1.0f, 1.0f, 1.f);
    cube.add(v);
    v.position = vec4(-1.0f, 1.0f, -1.0f, 1.f);
    cube.add(v);
}

// from the GLTools library (OpenGL Superbible gltMakeSphere())
template<typename Vertex>
void
sphere(Mesh<Vertex> &sphereBatch, math::real fRadius, int iSlices, int iStacks)
{
    using namespace math;
    real drho = 3.141592653589_r / real(iStacks);
    real dtheta = 2.0_r * 3.141592653589_r / static_cast<real>(iSlices);
    int i, j; // Looping variables

    for (i = 0; i < iStacks; i++) {
        real rho = static_cast<real>(i) * drho;
        real srho = math::sin(rho);
        real crho = math::cos(rho);
        real srhodrho = math::sin(rho + drho);
        real crhodrho = math::cos(rho + drho);

        // Many sources of OpenGL sphere drawing code uses a triangle fan
        // for the caps of the sphere. This however introduces texturing
        // artifacts at the poles on some OpenGL implementations

        vec3 vVertex[4];
        vec3 vNormal[4];
        //        vec2 vTexture[4];

        for (j = 0; j < iSlices; j++) {
            auto theta = (j == iSlices) ? 0.0_r : static_cast<real>(j) * dtheta;
            auto stheta = -math::sin(theta);
            auto ctheta = math::cos(theta);

            auto x = stheta * srho;
            auto y = ctheta * srho;
            auto z = crho;

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

            theta =
              ((j + 1) == iSlices) ? 0.0_r : static_cast<real>(j + 1) * dtheta;
            stheta = -math::sin(theta);
            ctheta = math::cos(theta);

            x = stheta * srho;
            y = ctheta * srho;
            z = crho;

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

            for (uint32_t k = 0; k < 3; ++k) {
                Vertex v;
                setPoint(v.position, vVertex[k]);
                setVec(v.normal, vNormal[k]);
                sphereBatch.addVertex(v);
            }

            // Rearrange for next triangle
            vVertex[0] = vVertex[1];
            vNormal[0] = vNormal[1];
            // vTexture[0] = vTexture[1];

            vVertex[1] = vVertex[3];
            vNormal[1] = vNormal[3];
            // vTexture[1] = vTexture[3];

            for (uint32_t k = 0; k < 3; ++k) {
                Vertex v;
                setPoint(v.position, vVertex[k]);
                setVec(v.normal, vNormal[k]);
                sphereBatch.addVertex(v);
            }
        }
    }

    sphereBatch.drawType(DrawArrays);
    sphereBatch.primType(GL_TRIANGLES);
}

} // namespace primitives

} // namespace glt

#endif
