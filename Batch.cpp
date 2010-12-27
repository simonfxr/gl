#include <cstring>

#include <GL/glew.h>

#include "defs.h"
#include "Batch.hpp"
#include "gltools.hpp"


namespace gltools {

static const uint32 MIN_SIZE = 4;

Batch::Batch(GLenum _primType, Batch::Attribute attrs) :
    primType(_primType),
    frozen(false),
    vertex_buffer(0),
    normal_buffer(0),
    color_buffer(0),
    data(attrs, MIN_SIZE)
{}

Batch::~Batch() {
    GLuint buffs[] = { vertex_buffer, normal_buffer, color_buffer };
    glDeleteBuffers(ARRAY_LENGTH(buffs), buffs);
}

namespace {

template <typename T>
T *resizeArr(T *in, uint32 size, uint32 newsize) {
    T *out = new T[newsize];
    memcpy(out, in, sizeof in[0] * size);
    delete [] in;
    return out;
}

}

Batch::Data::Data(Batch::Attribute attrs, uint32 initialSize) :
    attribs(attrs),
    filled(0),
    size(initialSize),
    vertices(0),
    normals(0),
    colors(0)
{
    if (hasAttr(Batch::Vertex))
        vertices = new vec3[size];
    if (hasAttr(Batch::Normal))
        normals = new vec3[size];
    if (hasAttr(Batch::Color))
        colors = new vec4[size];
}

Batch::Data::~Data() {
    delete [] vertices;
    delete [] normals;
    delete [] colors;
}

void Batch::Data::resize(uint32 s) {
    if (hasAttr(Batch::Vertex))
        vertices = resizeArr(vertices, size, s);
    if (hasAttr(Batch::Normal))
        normals = resizeArr(normals, size, s);
    if (hasAttr(Batch::Color))
        colors = resizeArr(colors, size, s);
    size = s;
}

void Batch::Data::vertex(const vec3& v) {
    if (filled >= size)
        resize(size * 2);
    vertices[filled++] = v;
}

void Batch::Data::normal(const vec3& n) {
    if (filled >= size)
        resize(size * 2);
    normals[filled] = n;
}

void Batch::Data::color(const vec4& c) {
    if (filled >= size)
        resize(size * 2);
    colors[filled] = c;
}

namespace {

template <typename T>
void sendData(GLuint *bufName, T * data, uint32 size) {
    *bufName = 0;
    glGenBuffers(1, bufName);
    glBindBuffer(GL_ARRAY_BUFFER, *bufName);
    glBufferData(GL_ARRAY_BUFFER, sizeof data[0] * size, data, GL_STATIC_DRAW);
}

}

void Batch::freeze() {
    frozen = true;

    if (data.hasAttr(Vertex)) {
        sendData(&vertex_buffer, data.vertices, data.filled);
        delete [] data.vertices;
        data.vertices = 0;
    }
    
    if (data.hasAttr(Normal)) {
        sendData(&normal_buffer, data.normals, data.filled);
        delete [] data.normals;
        data.normals = 0;
    }
    
    if (data.hasAttr(Color)) {
        sendData(&color_buffer, data.colors, data.filled);
        delete [] data.colors;
        data.colors = 0;
    }
}

void Batch::draw() const {

    ASSERT(frozen, "cannot draw batch while building");

//    GL_CHECK(glBindVertexArray(vertex_buffer));
    
    if (data.hasAttr(Vertex)) {
        GL_CHECK(glEnableVertexAttribArray(VertexPos));
        GL_CHECK(glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer));
        GL_CHECK(glVertexAttribPointer(VertexPos, 3, GL_FLOAT, GL_FALSE, 0, 0));
    }

    if (data.hasAttr(Normal)) {
        GL_CHECK(glEnableVertexAttribArray(NormalPos));
        GL_CHECK(glBindBuffer(GL_ARRAY_BUFFER, normal_buffer));
        GL_CHECK(glVertexAttribPointer(NormalPos, 3, GL_FLOAT, GL_FALSE, 0, 0));
    }
    
    if (data.hasAttr(Color)) {
        GL_CHECK(glEnableVertexAttribArray(ColorPos));
        GL_CHECK(glBindBuffer(GL_ARRAY_BUFFER, color_buffer));
        GL_CHECK(glVertexAttribPointer(ColorPos, 4, GL_FLOAT, GL_FALSE, 0, 0));
    }
    
    GL_CHECK(glDrawArrays(primType, 0, data.filled));
    
    GL_CHECK(glDisableVertexAttribArray(VertexPos));
    GL_CHECK(glDisableVertexAttribArray(NormalPos));
    GL_CHECK(glDisableVertexAttribArray(ColorPos));
}

} // namespace gltools
