#ifndef _VEC3_H
#define _VEC3_H

#include "Vec.hpp"
#include "Math.hpp"

template <typename T> struct Vec3;

template <typename T>
struct Vec3 : public Vec<3, T, Vec3<T>, Math> {

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
};
#endif
