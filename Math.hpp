#ifndef _MATH_HPP
#define _MATH_HPP

#include "defs.h"
#include <cmath>

class Math {
public:

    static real32 sqrt(real32 x) {
        return sqrtf(x);
    }

    static real32 rsqrt(real32 x) {
        return recp(sqrt(x));
    }

    static real32 recp(real32 x) {
        return x == 0.f ? 0.f : 1 / x;
    }

    class Fast {
    public:
        
        static real32 sqrt(real32 x) {
            real32 y;
            __asm__ ("sqrtss %1, %0" : "=x"(y) : "x"(x));
            return y;
        }

        static real32 recp(real32 x) {
            real32 y;
            __asm__ ("rcpss %1, %0" : "=x"(y) : "x"(x));
            return y;
        }

        static real32 rsqrt(real32 x) {
            real32 y;
            __asm__ ("rsqrtss %1, %0" : "=x"(y) : "x"(x));
            return y;
        }
        
    private:
        Fast();
        ~Fast();
    };

private:
    Math();
    ~Math();
};
#endif
