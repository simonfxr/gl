#ifndef GENBATCH_HPP
#define GENBATCH_HPP

#include <GL/glew.h>
#include <GL/gl.h>

#include "defs.h"
#include "gltools.hpp"

struct vec3;
struct vec4;
struct color;

namespace gltools {

struct Attr {
    GLint size;
    GLenum type;
    GLboolean normalized;
    const GLvoid *offset;
};

template <typename T>
struct Attrs {  
    uint32 length;
    const Attr *attrs;

    Attrs(uint32 _length, const Attr _attrs[])
        : length(_length), attrs(_attrs) {}

    GLuint index(uint64 offset);
};

template <typename T>
GLuint Attrs<T>::index(uint64 offset) {
    const GLvoid *off = (const GLvoid *) offset;
    for (uint32 i = 0; i < length; ++i)
        if ((const GLvoid *) off == attrs[i].offset)
            return i;
    FATAL_ERROR("unknown offset in attribute list");
}

namespace {

namespace attr {

Attr LOCAL vec3(uint64 offset) {
    Attr a;
    a.size = 3; a.type = GL_FLOAT; a.normalized = GL_FALSE;
    a.offset = (const GLvoid *) offset;
    return a;
}

Attr LOCAL vec4(uint64 offset) {
    Attr a;
    a.size = 4; a.type = GL_FLOAT; a.normalized = GL_FALSE;
    a.offset = (const GLvoid *) offset;
    return a;
}

Attr LOCAL color(uint64 offset) {
    Attr a;
    a.size = 4; a.type = GL_UNSIGNED_BYTE; a.normalized = GL_TRUE;
    a.offset = (const GLvoid *) offset;
    return a;
}

static const Attr null = {
    0, GL_FLOAT, GL_FALSE, NULL
};

} // namespace attr

} // namespace anon

namespace priv {

struct DynBatch {
    uint32 size;
    uint32 filled;
    
    GLuint *buffer_names;
    byte **data;

    uint32 nattrs;
    
    DynBatch(uint32 nattrs, const Attr attrs[], uint32 initialSize);
    ~DynBatch();
    
    void add(const Attr attrs[], const void *value);
    void send(const Attr attrs[]);
    void draw(const Attr attrs[], GLenum primType, bool enabled[]);
};

} // namespace priv

template <typename T>
struct GenBatch {
private:

    const GLenum primType;
    const Attrs<T>& attrs;
    bool frozen;
    priv::DynBatch batch;
    bool *enabledAttrs;
    
    GenBatch(const GenBatch& _);
    GenBatch& operator =(const GenBatch& _);

public:

    GenBatch(GLenum _primType, const Attrs<T>& _attrs) :
        primType(_primType),
        attrs(_attrs),
        frozen(false),
        batch(_attrs.length, _attrs.attrs, 4),
        enabledAttrs(new bool[_attrs.length])
    {
        for (uint32 i = 0; i < attrs.length; ++i)
            enabledAttrs[i] = true;
    }

    ~GenBatch() {
        delete [] enabledAttrs;
    }

    const Attrs<T>& getAttrs() { return attrs; }

    void add(const T& v) {
        ASSERT(!frozen, "cannot add to frozen batch");
        batch.add(attrs.attrs, static_cast<const void *>(&v));
    }

    void freeze() { frozen = true; }

    void draw() {
        ASSERT(frozen, "cannot draw batch while building it");
        batch.draw(attrs.attrs, primType, enabledAttrs);
    }

    void enableAttrib(GLuint position, bool enable = true) {
        enabledAttrs[position] = enable;
    }
};

} // namespace gltools
#endif
