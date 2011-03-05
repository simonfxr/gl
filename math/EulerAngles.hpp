#ifndef EULER_ANGLES_HPP
#define EULER_ANGLES_HPP

#include "math/math.hpp"
#include "math/mat4.hpp"
#include "math/vec4.hpp"

namespace math {

struct EulerAngles {

    float heading;
    float pitch;
    float bank;

    EulerAngles() {}

    EulerAngles(float _heading, float _pitch, float _bank) :
        heading(_heading), pitch(_pitch), bank(_bank) {}

    EulerAngles(const mat4_t& rot) {

	// Extract sin(pitch) from m23.

	float sp = -rot[2][1];

	// Check for Gimbel lock
	
	if (abs(sp) > 9.99999f) {

            // Looking straight up or down
            
            pitch = 0.5 * PI * sp;

            // Compute heading, slam bank to zero

            heading = atan2(-rot[0][2], rot[0][0]);
            bank = 0.0f;

	} else {

            // Compute angles.  We don't have to use the "safe" asin
            // function because we already checked for range errors when
            // checking for Gimbel lock
            
            heading = atan2(rot[2][0], rot[2][2]);
            pitch = asin(sp);
            bank = atan2(rot[0][1], rot[1][1]);
	}

    }

    void canonize() {

        pitch = wrapPi(pitch);

        if (pitch < -0.5f * PI) {
            pitch = -PI - pitch;
            heading += PI;
            bank += PI;
        } else if (pitch > 0.5f * PI) {
            pitch = PI - pitch;
            heading += PI;
            bank += PI;
        }

        if (abs(pitch) > 0.5 * PI - 1e-4) {
            heading += bank;
            bank = 0.f;
        } else {
            bank = wrapPi(bank);
        }

        heading = wrapPi(heading);
    }

    mat4_t getRotationMatrix() const {

        float sh, ch, sp, cp, sb, cb;
        sincos(heading, sh, ch);
        sincos(pitch, sp, cp);
        sincos(bank, sb, cb);

        return mat4(vec4(ch * cb + sh * sp * sb, sb * cp, -sh * cb + ch * sp * sb, 0.f),
                    vec4(-ch * sb + sh * cp * cb, cb * cp, sb * sh + ch * sp * cb, 0.f),
                    vec4(sh * cp, -sp, ch * cp, 0.f),
                    vec4(0.f, 0.f, 0.f, 1.f));
        
    }
};

}
#endif
