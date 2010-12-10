
#include "Batch.cpp"

#include <cstring>

static const uint32 MIN_SIZE = 4;

Batch::Batch(GLenum _primType, Batch::Attribute attrs) :
    primType(_primType),
    frozen(false),
    data(attrs, MIN_SIZE);
{}

Batch::~Batch() {
    delete [] vertices;
    delete [] normals;
    delete [] colors;
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
    vertices(0),
    normals(0),
    colors(0),
    filled(0),
    size(0)
{
    resize(initialSize);
}

Data::~Data() {
    delete [] vertices;
    delete [] normals;
    delete [] colors;
}

void Batch::Data::resize(uint32 s) {
    if ((attribs & Batch::Attribute::Vertex) != 0)
        vertices = resizeArr(vertices, size, s);
    if ((attribs & Batch::Attribute::Normal) != 0)
        normals = resizeArr(normals, size, s);
    if ((attribs & Batch::Attribute::Color) != 0)
        colors = resizeArr(colors, size, s);
    size = s;
}

void Batch::Data::vertex(const vec3& v) {
    if (filled >= size)
        resize(size * 2);
    vertices[filled++] = v;
}

void Batch::Data::normal(const vec3& n) {
    normals[filled] = n;
}

void Batch::Data::color(const vec4& c) {
    colors[filled] = c;
}

void Batch::freeze() {
    frozen = true;
}

void Batch::draw() const {

}
