#ifndef MATH_QUAT_DEFNS_HPP
#define MATH_QUAT_DEFNS_HPP

#include "math/mdefs.hpp"
#include "math/quat/type.hpp"
#include "math/vec4/type.hpp"
#include "math/vec3/type.hpp"

MATH_BEGIN_NAMESPACE

MATH_FUNC quat_t quat() PURE_FUNC;

MATH_FUNC quat_t quat(const vec4_t& coefficients) PURE_FUNC;

MATH_FUNC float length(const quat_t& q) PURE_FUNC;

MATH_FUNC float lengthSq(const quat_t& q) PURE_FUNC;

MATH_FUNC float inverseLength(const quat_t& q) PURE_FUNC;

MATH_FUNC quat1_t normalize(const quat_t& q) PURE_FUNC;

MATH_FUNC quat_t inverse(const quat_t& q) PURE_FUNC;

MATH_FUNC quat_t conjugate(const quat_t& q) PURE_FUNC;

MATH_FUNC quat_t operator -(const quat_t& p) PURE_FUNC;

MATH_FUNC quat_t operator *(const quat_t& p, const quat_t& q) PURE_FUNC;

MATH_FUNC quat_t& operator *=(quat_t& p, const quat_t& q) MUT_FUNC;

MATH_FUNC quat1_t pow(const quat1_t& q, float exp) PURE_FUNC;

MATH_FUNC quat1_t slerp(const quat1_t& p, const quat1_t& q, float t) PURE_FUNC;

MATH_END_NAMESPACE

#endif
