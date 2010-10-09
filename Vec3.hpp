#ifndef _VEC3_H
#define _VEC3_H

#include "Vec.hpp"
#include "Math.hpp"

template <typename T> struct Vec3;

template <typename T>
struct Vec3 : public Vec<3, T, Vec3<T>> {

    Vec3(T _x, T _y, T _z) {
        this->data[0] = _x;
        this->data[1] = _y;
        this->data[2] = _z;
    }

    Vec3() {
        this->data[0] = 0;
        this->data[1] = 0;
        this->data[2] = 0;
    }

#define defAccessor(name, e)                    \
    T name() const { return e; }                \
    T& name() { return e; }

    defAccessor(x, data[0])

    defAccessor(y, data[1])

    defAccessor(z, data[2])

#undef defAccessor
    
    Vec3 cross(Vec3& B) const {
        const Vec3& A = *this;
        V C;
        C.x() = A.y() * B.z() - A.z() * B.y();
        C.y() = A.z() * B.x() - A.x() * B.z();
        C.z() = A.x() * B.y() - A.y() * B.x();
        return C;
    }
};

typedef Vec3<real32> V3;
#endif
