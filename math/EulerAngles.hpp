#ifndef EULER_ANGLES_HPP
#define EULER_ANGLES_HPP

#include "math/Math.hpp"
#include "math/mat4.hpp"

struct EulerAngles {

    float heading;
    float pitch;
    float bank;

    EulerAngles() {}

    EulerAngles(float _heading, float _pitch, float _bank) :
        heading(_heading), pitch(_pitch), bank(_bank) {}

    void canonize() {

        pitch = Math::wrapPi(pitch);

        if (pitch < -0.5f * Math::PI) {
            pitch = -Math::PI - pitch;
            heading += Math::PI;
            bank += Math::PI;
        } else if (pitch > 0.5f * Math::PI) {
            pitch = Math::PI - pitch;
            heading += Math::PI;
            bank += Math::PI;
        }

        if (Math::abs(pitch) > 0.5 * Math::PI - 1e-4) {
            heading += bank;
            bank = 0.f;
        } else {
            bank = Math::wrapPi(bank);
        }

        heading = Math::wrapPi(heading);
    }

    mat4 getRotationMatrix() const {

        float sh, ch, sp, cp, sb, cb;
        Math::sincos(heading, &sh, &ch);
        Math::sincos(pitch, &sp, &cp);
        Math::sincos(bank, &sb, &cb);

        return mat4(vec4(ch * cb + sh * sp * sb, sb * cp, -sh * cb + ch * sp * sb, 0.f),
                    vec4(-ch * sb + sh * cp * cb, cb * cp, sb * sh + ch * sp * cb, 0.f),
                    vec4(sh * cp, -sp, ch * cp, 0.f),
                    vec4(0.f, 0.f, 0.f, 1.f));
        
    }
};


#endif
