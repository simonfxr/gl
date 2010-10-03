#ifndef _VEC_HPP
#define _VEC_HPP

#include "defs.h"

template <uint32 N, typename T, typename V, typename M>
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
               p += data[i] v.data[i];
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

     static V make(T val) {
          V r;
          for (uint32 i = 0; i < N; ++i)
               r.data[i] = val;
          return r;
     }
};
#endif
