#ifndef _VEC_HPP
#define _VEC_HPP

#include "defs.h"

template <uint32 N, typename T, typename V>
struct Vec {
    T data[N];

    V trans(const V& v) const {
        V r;
        for (uint32 i = 0; i < N; ++i)
            r.data[i] = data[i] + v.data[i];
        return r;
    }

    V scale(T k) const {
        V r;
        for (uint32 i = 0; i < N; ++i)
            r.data[i] = k * data[i];
    }

    V neg() const {
        V r;
        for (uint32 i = 0; i < N; ++i)
            r.data[i] = -data[i];
        return r;
    }

    T dot(const V& v) const {
        T p = 0;
        for (uint32 i = 0; i < N; ++i)
            p += data[i] * v.data[i];
        return p;
    }

    T magSq() const {
        return dot(*this);
    }

    T mag() const {
        return M::sqrt(magSq());
    }

    T rmag() const {
        return M::rsqrt(magSq());
    }

    V norm() const {
        return scale(rmag());
    }

    T operator ()(int i) const {
        return data[i];
    }

    T &operator ()(int i) const {
        return data[i];
    }

    V &operator -() const {
        return neg();
    }

    V operator +(const V& v) const {
        return trans(v);
    }

    V operator -(const V& v) const {
        return trans(v.neg());
    }

    V& operator +=(const V& v) const {
        *this = *this + v;
        return *this;
    }

    V& operator -=(const V& v) const {
        *this = *this - v;
        return *this;
    }

    V operator *(T s) const {
        return scale(s);
    }

    V operator /(T s) const {
        return scale(M::recp(s));
    }

    V& operator *=(T s) {
        *this = *this * s;
        return *this;
    }

    V& operator /=(T s) {
        *this = *this / s;
        return *this;
    }

    V& operator =(const V& v) {
        for (uint32 i = 0; i < N; ++i)
            data[i] = v.data[i];
        return *this;
    }

    static V make(T val) {
        V r;
        for (uint32 i = 0; i < N; ++i)
            r.data[i] = val;
        return r;
    }
};

namespace {
    template <uint32 N, typename T, typename V>
    inline V operator *(T x, const &V A) {
        return A * x;
    }
}
#endif
