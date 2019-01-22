#ifndef BL_EQUAL_HPP
#define BL_EQUAL_HPP

namespace bl {

template<typename T>
struct equal
{
    template<typename U>
    bool operator()(const T &a, const U &b) const
    {
        return a == b;
    }
};

} // namespace bl

#endif
