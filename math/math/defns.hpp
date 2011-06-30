#ifndef MATH_DEFNS_HPP
#define MATH_DEFNS_HPP

#include "defs.h"
#include "math/defs.hpp"

#include <limits>

MATH_BEGIN_NAMESPACE

static const float MATH_CONSTANT PI = 3.1415926;

static const float MATH_CONSTANT POS_INF = std::numeric_limits<float>::infinity();

static const float MATH_CONSTANT NEG_INF = - POS_INF;

float sqrt(float x) PURE_FUNC;

float recip(float x) PURE_FUNC;

float inverse(float x) PURE_FUNC;

float rsqrt(float x) PURE_FUNC;

float sin(float rad) PURE_FUNC;

float cos(float rad) PURE_FUNC;

float tan(float rad) PURE_FUNC;

float asin(float x) PURE_FUNC;

float acos(float x) PURE_FUNC;

float atan(float x) PURE_FUNC;

float atan2(float x, float y) PURE_FUNC;

void sincos(float rad, float& out_sin, float& out_cos) PURE_FUNC;

float rtan(float rad) PURE_FUNC;

float abs(float x) PURE_FUNC;

float length(float x) PURE_FUNC;

float distance(float x, float y) PURE_FUNC;

float squared(float x) PURE_FUNC;

float cubed(float x) PURE_FUNC;

float pow(float x, int32 n) PURE_FUNC;

float pow(float x, float y) PURE_FUNC;

int32 signum(float x) PURE_FUNC;

// bool signbit(float x) PURE_FUNC;

float wrapPi(float x) PURE_FUNC;

float wrap(float x, float period) PURE_FUNC;

float degToRad(float deg) PURE_FUNC;

float radToDeg(float rad) PURE_FUNC;

float max(float x, float y) PURE_FUNC;

float min(float x, float y) PURE_FUNC;

float saturate(float x) PURE_FUNC;

float clamp(float lo, float hi, float x) PURE_FUNC;

float smoothstep(float lo_edge, float hi_edge, float x) PURE_FUNC;

float lerp(float a, float b, float t) PURE_FUNC;

MATH_END_NAMESPACE

#endif
