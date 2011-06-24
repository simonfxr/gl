#ifndef GLT_PRIMITIVES_HPP
#define GLT_PRIMITIVES_HPP

#include "defs.h"
#include "error/error.hpp"

#include "math/vec3.hpp"
#include "math/ivec3.hpp"

#include "glt/CubeMesh.hpp"

namespace glt {

namespace primitives {

template <typename V>
void cubeoid(CubeMesh<V>& model, const math::point3_t& origin, const math::vec3_t dim, bool evert = false) {

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
void unitCube(CubeMesh<Vertex>& cube) {

    using namespace math;
    
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

} // namespace primitives

} // namespace glt

#endif
