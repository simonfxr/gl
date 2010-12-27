#ifndef BATCH2_HPP
#define BATCH2_HPP

#include <GL/gl.h>

#include "defs.h"
#include "GenBatch.hpp"
#include "math/vec3.hpp"
#include "color.hpp"

namespace gltools {

struct BatchVertex {
    vec3 position;
    vec3 normal;
    gltools::color color;
};

struct Batch {

    enum Attribute {
        Vertex = 1,
        Normal = 2,
        Color  = 4
    };

    enum AttributePosition {
        VertexPos = 0,
        NormalPos = 1,
        ColorPos  = 2
    };

private:

    static const Attrs<BatchVertex> vertexAttrs;
    
    GenBatch<BatchVertex> batch;
    BatchVertex building;
    Attribute attribs;

    bool hasAttr(Attribute a) {
        return (attribs & a) != 0;
    }

public:

    explicit Batch(GLenum primType, Attribute attribsToEnable = Vertex) :
        batch(primType, vertexAttrs),
        attribs(attribsToEnable)
    {
        batch.enableAttrib(VertexPos, hasAttr(Vertex));
        batch.enableAttrib(NormalPos, hasAttr(Normal));
        batch.enableAttrib(ColorPos, hasAttr(Color));        
    }
    
    void freeze() { batch.freeze(); }
    void draw() { batch.draw(); }

    void vertex(const vec3& v) { building.position = v; batch.add(building); }
    void normal(const vec3& n) { building.normal = n; }
    void color(const vec4& c) { building.color = gltools::color(c); }
};

inline Batch::Attribute operator |(Batch::Attribute a1, Batch::Attribute a2) {
    return static_cast<Batch::Attribute>(int64(a1) | int64(a2));
}

const Attr vertexAttrArray[] = {
    gltools::attr::vec3(offsetof(BatchVertex, position)),
    gltools::attr::vec3(offsetof(BatchVertex, normal)),
    gltools::attr::color(offsetof(BatchVertex, color))
};

const Attrs<BatchVertex> Batch::vertexAttrs(ARRAY_LENGTH(vertexAttrArray), vertexAttrArray);

} // namespace gltools

#endif
