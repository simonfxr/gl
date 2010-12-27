#include <iostream>

#include "v4.hpp"

std::ostream &operator <<(std::ostream& out, const v4::v4& v) {
    return out << "[" << v4::v4a(v) << "; " << v4::v4b(v) << "; " << v4::v4c(v) << "]";
}

int main(void) {

    const v4::v4 A = v4::make3(7, 8, 9);
    const v4::v4 B = v4::make3(4, 5, 6);

#define op2(op) std::cout << #op << "(" << A << ", " << B << ") = " << v4::op(A, B) << std::endl
#define op1(op) std::cout << #op << "(" << A << ") = " << v4::op(A) << std::endl

    op2(add3);
    op2(sub3);
    op2(mul3);
    op2(dot3);
    op1(neg3);
    op1(horiadd3);

    return 0;
}
