#ifndef BATCH_HPP
#define BATCH_HPP

#include <GL/gl.h>

#include "defs.h"

namespace gltools {

struct Batch {

    enum Attribute {
        Vertex = 1,
        Normal = 2,
        Color  = 4
    };

private:

    const GLenum primType;

    bool frozen;

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
    };

    // to let the compiler optimise away accept() calls, we have
    // to avoid reloads of the frozen field between calls to
    // the vertex/normal/color methods, in order to do that we collect
    // all fields that are needed by these methods,
    // so the compiler can see that the frozen flag cannot be changed in these methods.
    Data data;

    Batch(const Batch& _);
    Batch& operator =(const Batch& _);

    bool accept(Attribute dest) {
        return !frozen && (attribs & dest) != 0;
    }

public:

    explicit Batch(GLenum primType, Attribute attribsToEnable = Attribute::Vertex);
    ~Batch();

    void freeze();
    void draw() const;

    void vertex(const vec3& v) { if (accept(Attribute::Vertex)) data.vertex(n); }
    void normal(const vec3& n) { if (accept(Attribute::Normal)) data.normal(n); }
    void color(const vec4& c) { if (accept(Attribute::Color)) data.color(c); }
};


} // namespace gltools


#endif
