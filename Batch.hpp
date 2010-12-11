#ifndef BATCH_HPP
#define BATCH_HPP

#include <GL/gl.h>

#include "defs.h"
#include "math/vec3.hpp"
#include "math/vec4.hpp"

namespace gltools {

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

    const GLenum primType;

    bool frozen;

    GLuint vertex_buffer;
    GLuint normal_buffer;
    GLuint color_buffer;

    struct Data {
        const Attribute attribs;

        uint32 filled;
        uint32 size;
        
        vec3 *vertices;
        vec3 *normals;
        vec4 *colors;

        Data(Attribute attribs, uint32 initialSize);
        ~Data();

        void vertex(const vec3& v);
        void normal(const vec3& n);
        void color(const vec4& c);

        void resize(uint32 newsize);

        bool hasAttr(Attribute dest) const {
            return (attribs & dest) != 0;
        }
    };

    // to let the compiler optimise away accept() calls, we have
    // to avoid reloads of the frozen field between calls to
    // the vertex/normal/color methods, in order to do that we collect
    // all fields that are needed by these methods,
    // so the compiler can see that the frozen flag cannot be changed in these methods.
    Data data;

    Batch(const Batch& _);
    Batch& operator =(const Batch& _);

    bool accept(Attribute dest) const {
        return !frozen && data.hasAttr(dest);
    }

public:

    explicit Batch(GLenum primType, Attribute attribsToEnable = Vertex);
    ~Batch();

    void freeze();
    void draw() const;

    void vertex(const vec3& v) { if (accept(Vertex)) data.vertex(v); }
    void normal(const vec3& n) { if (accept(Normal)) data.normal(n); }
    void color(const vec4& c) { if (accept(Color)) data.color(c); }
};
    
inline Batch::Attribute operator |(Batch::Attribute a1, Batch::Attribute a2) {
    return static_cast<Batch::Attribute>(int64(a1) | int64(a2));
}

} // namespace gltools


#endif
